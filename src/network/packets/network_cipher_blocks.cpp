#include "nfs/network/packets/network_cipher.hpp"
namespace nfs::network::cipher {
namespace {
Dword left(Dword v) {
    return v << 1 | v >> 31;
}
Dword right(Dword v) {
    return v >> 1 | v << 31;
}
Dword first(const Dword *table, Dword v) {
    return table[v & 255] ^ table[256 + (v >> 8 & 255)] ^ table[512 + (v >> 16 & 255)] ^
           table[768 + (v >> 24)];
}
Dword second(const Dword *table, Dword v) {
    return table[256 + (v & 255)] ^ table[512 + (v >> 8 & 255)] ^ table[768 + (v >> 16 & 255)] ^
           table[v >> 24];
}
} // namespace
// Sixteen cipher rounds.
void encrypt_block(State *state, const Dword *input, Dword *output) {
    const Dword *key = state->schedule, *table = state->tables;
    Dword a = input[0] ^ key[0], b = input[1] ^ key[1], c = input[2] ^ key[2], d = input[3] ^ key[3];
    for (unsigned round = 0; round < 8; ++round) {
        Dword x = first(table, a), y = second(table, b);
        c = right(c ^ (x + y + key[8 + round * 4]));
        d = left(d) ^ (x + y * 2 + key[9 + round * 4]);
        x = first(table, c);
        y = second(table, d);
        a = right(a ^ (x + y + key[10 + round * 4]));
        b = left(b) ^ (x + y * 2 + key[11 + round * 4]);
    }
    output[1] = d ^ key[5];
    output[0] = c ^ key[4];
    output[2] = a ^ key[6];
    output[3] = b ^ key[7];
}

void decrypt_block(State *state, const Dword *input, Dword *output) {
    const Dword *key = state->schedule, *table = state->tables;
    Dword a = input[0] ^ key[4], b = input[1] ^ key[5], c = input[2] ^ key[6], d = input[3] ^ key[7];
    for (int round = 7; round >= 0; --round) {
        Dword x = first(table, a), y = second(table, b);
        d = right(d ^ (x + y * 2 + key[11 + round * 4]));
        c = left(c) ^ (x + y + key[10 + round * 4]);
        x = first(table, c);
        y = second(table, d);
        b = right(b ^ (x + y * 2 + key[9 + round * 4]));
        a = left(a) ^ (x + y + key[8 + round * 4]);
    }
    output[0] = c ^ key[0];
    output[3] = b ^ key[3];
    output[1] = d ^ key[1];
    output[2] = a ^ key[2];
}

void __fastcall encrypt_in_place(State *state, void *, Dword *data) {
    encrypt_block(state, data, data);
}

void __fastcall decrypt_in_place(State *state, void *, Dword *data) {
    decrypt_block(state, data, data);
}

Dword __fastcall block_bytes(State *, void *) {
    return 16;
}
} // namespace nfs::network::cipher
