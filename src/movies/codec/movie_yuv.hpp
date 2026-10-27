#pragma once
#include "../playback/movie_runtime.hpp"
#include <cstring>
namespace nfs::movies::yuv {
extern std::int16_t luminance[256][4], chroma_u[256][4], chroma_v[256][4];
extern std::uint64_t multiplier, green_mask, dither, dither_flip, component_mask;
extern Dword component_order;
extern Byte filter_rows[8208];
template <class T> T load(const void *input) {
    T value;
    std::memcpy(&value, input, sizeof(T));
    return value;
}
template <class T> void store(void *output, T value) {
    std::memcpy(output, &value, sizeof(T));
}
template <class T> T *advance(T *pointer, Dword bytes) {
    return reinterpret_cast<T *>(reinterpret_cast<Dword>(pointer) + bytes);
}
inline Dword half(Dword colour) {
    return (colour >> 1) & 2139062143;
}
inline Dword add_bytes(Dword first, Dword second) {
    Dword output = 0;
    for (unsigned lane = 0; lane < 4; ++lane)
        output |= Dword(Byte((first >> (lane * 8)) + (second >> (lane * 8)))) << (lane * 8);
    return output;
}
// PADDW and PACKUSWB semantics on the four colour lanes; no floating-point conversion.
inline Dword colour(Byte luma, Byte u, Byte v) {
    Dword output = 0;
    for (unsigned lane = 0; lane < 4; ++lane) {
        std::uint16_t chroma = std::uint16_t(Dword(std::uint16_t(chroma_u[u][lane])) +
                                             Dword(std::uint16_t(chroma_v[v][lane])));
        std::int16_t value =
            std::int16_t(std::uint16_t(Dword(std::uint16_t(luminance[luma][lane])) + chroma));
        output |= Dword(value < 0 ? 0 : value > 255 ? 255 : value) << (lane * 8);
    }
    return output;
}
inline Dword pack(std::uint64_t colours, std::uint64_t bias, std::uint64_t mask,
                 std::uint64_t factors, std::uint64_t green) {
    std::uint64_t saturated = 0;
    for (unsigned lane = 0; lane < 8; ++lane) {
        unsigned value =
            unsigned((colours >> (lane * 8)) & 255) + unsigned((bias >> (lane * 8)) & 255);
        saturated |= std::uint64_t(value > 255 ? 255 : value) << (lane * 8);
    }
    const std::uint64_t components = saturated & mask;
    std::uint64_t product = 0;
    for (unsigned pixel = 0; pixel < 2; ++pixel) {
        unsigned shift = pixel * 32;
        auto low = std::int16_t(components >> shift),
             high = std::int16_t(components >> (shift + 16));
        auto low_factor = std::int16_t(factors >> shift),
             high_factor = std::int16_t(factors >> (shift + 16));
        Dword value = Dword(int(low) * low_factor + int(high) * high_factor);
        Dword green_bits = Dword((saturated & green) >> shift) >> 11;
        product |= std::uint64_t(value | green_bits) << shift;
    }
    return Dword(product | (product >> 16));
}
void set_component_order(Dword);
void configure_packed(Dword);
void row_yuy2(const Byte *, const Byte *, const Byte *, Byte *, Byte *);
void row_colour(const Byte *, const Byte *, const Byte *, Byte *, Byte *);
void row_packed(const Byte *, const Byte *, const Byte *, Byte *, Byte *);
void row_doubled(const Byte *, const Byte *, const Byte *, Byte *, Byte *);
void row_horizontal(const Byte *, Byte *, Byte *);
void row_vertical(const Byte *, const Byte *, Byte *, Byte *);
} // namespace nfs::movies::yuv
