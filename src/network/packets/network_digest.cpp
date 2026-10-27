#include "nfs/network/packets/network_digest.hpp"
#include <cstring>
#include <cstdio>
namespace nfs::network::digest {
namespace {
Dword rotate(Dword value, unsigned count) {
    return (value << count) | (value >> (32 - count));
}
void forward_copy(void *destination, const void *source, Dword bytes) {
    auto *out = static_cast<std::uint8_t *>(destination);
    auto *in = static_cast<const std::uint8_t *>(source);
    Dword index = 0;
    for (; index < (bytes & ~3u); index += 4)
        *reinterpret_cast<Dword *>(out + index) = *reinterpret_cast<const Dword *>(in + index);
    for (; index < bytes; ++index)
        out[index] = in[index];
}
} // namespace
// The 64 saved additions/rotations, expressed as four rounds.
void transform(State *state, const void *source) {
    static constexpr Dword constants[] = {
        3614090360u, 3905402710u, 606105819, 3250441966u, 4118548399u, 1200080426, 2821735955u,
        4249261313u, 1770035416, 2336552879u, 4294925233u, 2304563134u, 1804603682, 4254626195u,
        2792965006u, 1236535329, 4129170786u, 3225465664u, 643717713, 3921069994u, 3593408605u,
        38016083, 3634488961u, 3889429448u, 568446438, 3275163606u, 4107603335u, 1163531501,
        2850285829u, 4243563512u, 1735328473, 2368359562u, 4294588738u, 2272392833u, 1839030562,
        4259657740u, 2763975236u, 1272893353, 4139469664u, 3200236656u, 681279174, 3936430074u,
        3572445317u, 76029189, 3654602809u, 3873151461u, 530742520, 3299628645u, 4096336452u,
        1126891415, 2878612391u, 4237533241u, 1700485571, 2399980690u, 4293915773u, 2240044497u,
        1873313359, 4264355552u, 2734768916u, 1309151649, 4149444226u, 3174756917u, 718787259,
        3951481745u};
    static constexpr unsigned shifts[4][4] = {
        {7, 12, 17, 22}, {5, 9, 14, 20}, {4, 11, 16, 23}, {6, 10, 15, 21}};
    Dword message[16];
    auto *bytes = static_cast<const std::uint8_t *>(source);
    for (unsigned i = 0; i < 16; ++i)
        message[i] = Dword(bytes[i * 4]) | (Dword(bytes[i * 4 + 1]) << 8) |
                     (Dword(bytes[i * 4 + 2]) << 16) | (Dword(bytes[i * 4 + 3]) << 24);
    Dword a = state->hash[0], b = state->hash[1], c = state->hash[2], d = state->hash[3];
    for (unsigned step = 0; step < 64; ++step) {
        Dword combine;
        unsigned word;
        if (step < 16) {
            combine = (b & c) | (~b & d);
            word = step;
        } else if (step < 32) {
            combine = (d & b) | (~d & c);
            word = (5 * step + 1) & 15;
        } else if (step < 48) {
            combine = b ^ c ^ d;
            word = (3 * step + 5) & 15;
        } else {
            combine = c ^ (b | ~d);
            word = (7 * step) & 15;
        }
        Dword previous = d;
        d = c;
        c = b;
        b += rotate(a + combine + constants[step] + message[word], shifts[step / 16][step % 4]);
        a = previous;
    }
    state->hash[0] += a;
    state->hash[1] += b;
    state->hash[2] += c;
    state->hash[3] += d;
}
// Includes the saved partial-block and bit-count carry behavior.
void update(State *state, const void *source, Dword bytes) {
    Dword offset = (state->bits_low >> 3) & 63;
    const Dword added = bytes * 8;
    state->bits_low += added;
    if (state->bits_low < added)
        ++state->bits_high;
    state->bits_high += bytes >> 29;
    Dword consumed = 64 - offset;
    auto *input = static_cast<const std::uint8_t *>(source);
    if (bytes < consumed)
        consumed = 0;
    else {
        forward_copy(state->block + offset, input, consumed);
        transform(state, state->block);
        for (Dword last = 127 - offset; last < bytes; last += 64) {
            transform(state, input + last - 63);
            consumed += 64;
        }
        offset = 0;
    }
    forward_copy(state->block + offset, input + consumed, bytes - consumed);
}
// Lowercase "%02x" formatting and 88-byte state clearing.
void hexadecimal(const void *source, Dword bytes, char *output) {
    State state;
    state.hash[0] = 1732584193;
    state.hash[1] = 4023233417u;
    state.hash[2] = 2562383102u;
    state.hash[3] = 271733878;
    state.bits_low = state.bits_high = 0;
    update(&state, source, bytes);
    Dword length[2] = {state.bits_low, state.bits_high};
    static const std::uint8_t padding[64] = {128};
    const Dword offset = (state.bits_low >> 3) & 63;
    update(&state, padding, (offset > 55 ? 120 : 56) - offset);
    update(&state, length, 8);
    std::uint8_t result[16];
    std::memcpy(result, state.hash, 16);
    std::memset(&state, 0, sizeof(state));
    for (unsigned index = 0; index < 16; ++index)
        std::sprintf(output + index * 2, "%02x", result[index]);
}
} // namespace nfs::network::digest
