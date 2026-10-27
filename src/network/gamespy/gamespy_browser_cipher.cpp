#include "gamespy_browser.hpp"
namespace nfs::network::session::pending::browser {
namespace {
Dword rotate_left_byte(Dword value) {
    return value << 8 | value >> 24;
}
Dword rotate_right_byte(Dword value) {
    return value >> 8 | value << 24;
}
} // namespace

void encode_validation(const Byte *input, int length, char *output) {
    // Both routines have the same three-byte groups and unpadded alphabet.
    encode_query_validation(input, length, output);
}

void encrypt_validation(const Byte *key, int key_length, Byte *data, int length) {
    encrypt_query_validation(key, key_length, data, length);
}

Dword seed_cipher(Cipher *cipher, Dword seed, Dword route) {
    Dword second = 0, first = seed, branch = 1;
    cipher->depth = 0;
    for (Dword mask = 32768; mask; mask >>= 1) {
        second += first + branch;
        first += branch + second;
        if (!(route & mask)) {
            cipher->first_stack[cipher->depth] = first;
            cipher->second_stack[cipher->depth] = second;
            cipher->branch_stack[cipher->depth++] = branch;
            first ^= cipher->table[first & 255];
            second = rotate_right_byte(second) ^ cipher->table[(second >> 8) & 255];
            first = rotate_left_byte(first) ^ cipher->table[first >> 24];
            second = rotate_right_byte(second) ^ cipher->table[(second >> 8) & 255];
            first = rotate_left_byte(first);
            branch *= 2;
        } else {
            first = ~first;
            first = rotate_right_byte(first) ^ cipher->table[(first >> 8) & 255];
            second ^= cipher->table[second & 255];
            first = rotate_right_byte(first) ^ cipher->table[(first >> 8) & 255];
            second = rotate_left_byte(second) ^ cipher->table[second >> 24];
            branch = branch * 2 + 1;
            second = rotate_left_byte(second);
        }
    }
    cipher->branch = branch;
    cipher->seed = seed;
    cipher->second = second;
    cipher->first = first;
    return first ^ second;
}

void generate_cipher_words(Cipher *cipher, Dword *output, int count) {
    Dword first = cipher->first, branch = cipher->branch, second = cipher->second;
    int depth = cipher->depth;
    Dword *end = output + count;
    while (output < end) {
        while (branch < 65536) {
            second += first + branch;
            first += branch + second;
            cipher->second_stack[depth] = second;
            cipher->first_stack[depth] = first;
            cipher->branch_stack[depth++] = branch;
            first ^= cipher->table[first & 255];
            second = rotate_right_byte(second) ^ cipher->table[(second >> 8) & 255];
            branch *= 2;
            first = rotate_left_byte(first) ^ cipher->table[first >> 24];
            second = rotate_right_byte(second) ^ cipher->table[(second >> 8) & 255];
            first = rotate_left_byte(first);
        }
        *output++ = first ^ second;
        if (--depth < 0)
            depth = 0;
        branch = cipher->second_stack[depth];
        first = ~cipher->first_stack[depth];
        first = rotate_right_byte(first) ^ cipher->table[(first >> 8) & 255];
        branch ^= cipher->table[branch & 255];
        first = rotate_right_byte(first) ^ cipher->table[(first >> 8) & 255];
        branch = rotate_left_byte(branch) ^ cipher->table[branch >> 24];
        second = rotate_left_byte(branch);
        branch = cipher->branch_stack[depth] * 2 + 1;
    }
    cipher->branch = branch;
    cipher->depth = depth;
    cipher->first = first;
    cipher->second = second;
}

void initialize_cipher(const Byte *key, Dword key_length, Cipher *cipher) {
    cipher->cursor = nullptr;
    std::memset(cipher->table, 0, sizeof(cipher->table));
    for (Dword pass = 0; pass < 4; ++pass) {
        for (Dword index = 0; index < 256; ++index)
            cipher->table[index] = cipher->table[index] * 256 + index;
        Dword swap_index = pass;
        for (int repeat = 0; repeat < 2; ++repeat)
            for (Dword index = 0; index < 256; ++index) {
                const Dword held = cipher->table[index];
                swap_index = (key[index % key_length] + held + swap_index) & 255;
                cipher->table[index] = cipher->table[swap_index];
                cipher->table[swap_index] = held;
            }
    }
    for (Dword index = 0; index < 256; ++index)
        cipher->table[index] ^= index;
    seed_cipher(cipher, 0, 0);
}

void decrypt(Cipher *cipher, Byte *data, int size) {
    for (int index = 0; index < size; ++index) {
        if (!cipher->cursor || Dword(cipher->cursor - cipher->buffer) > 62) {
            cipher->cursor = cipher->buffer;
            generate_cipher_words(cipher, reinterpret_cast<Dword *>(cipher->buffer), 16);
        }
        data[index] ^= *cipher->cursor++;
    }
}
} // namespace nfs::network::session::pending::browser
