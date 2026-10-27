#include "nfs/network/packets/network_cipher.hpp"
#include <cstring>
#include <exception>
namespace nfs::network::cipher {
namespace {
Dword rotate(Dword value, unsigned bits) {
    return (value << bits) | (value >> (32 - bits));
}
Dword substitute(Dword value, unsigned a, unsigned b, unsigned c, unsigned d) {
    return Dword(substitutions[a][value & 255]) | Dword(substitutions[b][value >> 8 & 255]) << 8 |
           Dword(substitutions[c][value >> 16 & 255]) << 16 |
           Dword(substitutions[d][value >> 24]) << 24;
}
Dword matrix(Dword value) {
    return matrix_tables[0][value & 255] ^ matrix_tables[1][value >> 8 & 255] ^
           matrix_tables[2][value >> 16 & 255] ^ matrix_tables[3][value >> 24];
}
} // namespace
// Both shift registers and polynomial masks are retained.
Dword rs_remainder(Dword first, Dword second) {
    for (unsigned round = 0; round < 8; ++round) {
        const Dword byte = first >> 24;
        const Dword doubled = (byte * 2) ^ ((byte & 128) ? 333 : 0);
        const Dword mixed = (byte >> 1) ^ ((byte & 1) ? 166 : 0) ^ doubled;
        const Dword feedback = (((mixed << 8) | doubled) << 8 | mixed) << 8 | byte;
        first = (first << 8) ^ feedback ^ (second >> 24);
        second <<= 8;
    }
    return first;
}

Dword key_word(Dword value, const Dword *key, Dword cycles) {
    Dword word = value | (((value << 8 | value) << 8 | value) << 8);
    if (cycles == 4)
        word = substitute(word, 1, 0, 0, 1) ^ key[6];
    if (cycles == 4 || cycles == 3)
        word = substitute(word, 1, 1, 0, 0) ^ key[4];
    if (cycles != 2 && cycles != 3 && cycles != 4)
        return word;
    word = substitute(word, 0, 1, 0, 1) ^ key[2];
    return substitute(word, 0, 0, 1, 1) ^ key[0];
}
// Preserve key padding, temporary buffers and table order.
State *construct_state(State *state, const void *key, Dword bytes) {
    state->methods = state_initial_methods;
    const Dword cycles = bytes <= 16 ? 2 : bytes <= 24 ? 3 : 4;
    Dword *padded = nullptr, *rs = nullptr;
    int constructed = 0;
    try {
        state->schedule_count = 40;
        state->schedule = static_cast<Dword *>(gui::application_allocate(160));
        constructed = 1;
        state->table_blocks = 4;
        state->tables = static_cast<Dword *>(gui::application_allocate(4096));
        constructed = 2;
        state->methods = state_construct_methods;
        padded = static_cast<Dword *>(gui::application_allocate(cycles * 8));
        constructed = 3;
        std::memcpy(padded, key, bytes);
        std::memset(reinterpret_cast<Byte *>(padded) + bytes, 0, cycles * 8 - bytes);
        for (Dword index = 0; index < 40; index += 2) {
            const Dword first = matrix(key_word(index, padded, cycles));
            const Dword second = rotate(matrix(key_word(index + 1, padded + 1, cycles)), 8);
            state->schedule[index] = first + second;
            state->schedule[index + 1] = rotate(first + second * 2, 9);
        }
        rs = static_cast<Dword *>(gui::application_allocate(cycles * 8));
        constructed = 4;
        for (Dword index = 0; index < cycles; ++index)
            rs[(cycles - 1 - index) * 2] = rs_remainder(padded[index * 2 + 1], padded[index * 2]);
        for (Dword byte = 0; byte < 256; ++byte) {
            const Dword value = key_word(byte, rs, cycles);
            state->tables[byte] = matrix_tables[0][value & 255];
            state->tables[256 + byte] = matrix_tables[1][value >> 8 & 255];
            state->tables[512 + byte] = matrix_tables[2][value >> 16 & 255];
            state->tables[768 + byte] = matrix_tables[3][value >> 24];
        }
        std::memset(rs, 0, cycles * 8);
        gui::application_release(rs);
        constructed = 3;
        std::memset(padded, 0, cycles * 8);
        gui::application_release(padded);
        constructed = 2;
    } catch (...) {
        try {
            if (constructed >= 4) {
                std::memset(rs, 0, cycles * 8);
                lifecycle::construction::network::entries::release_payload(rs);
            }
            if (constructed >= 3) {
                std::memset(padded, 0, cycles * 8);
                lifecycle::construction::network::entries::release_payload(padded);
            }
            if (constructed >= 2) {
                std::memset(state->tables, 0, (state->table_blocks << 10) & ~Dword(3));
                lifecycle::construction::network::entries::release_payload(state->tables);
            }
            if (constructed >= 1) {
                std::memset(state->schedule, 0, (state->schedule_count & 1073741823) * 4);
                lifecycle::construction::network::entries::release_payload(state->schedule);
            }
            state->methods = state_base_methods;
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    return state;
}
} // namespace nfs::network::cipher
