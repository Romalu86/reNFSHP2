#include "movie_yuv.hpp"
#include <mmintrin.h>
#include <intrin.h>
namespace nfs::movies::yuv {
namespace {
__forceinline __m64 packed_words(const void *source) {
    const __m64 value = *reinterpret_cast<const __m64 *>(source);
    _ReadWriteBarrier();
    return value;
}
__forceinline void colour_words(const Byte *&luma, Byte u, Byte v, __m64 &left,
                               __m64 &right) {
    const __m64 first_chroma = packed_words(chroma_u[u]);
    const __m64 second_chroma = packed_words(chroma_v[v]);
    const __m64 chroma = _mm_add_pi16(first_chroma, second_chroma);
    const Byte first = luma[0], second = luma[1];
    luma += 2;
    const __m64 first_luma = packed_words(luminance[first]);
    const __m64 second_luma = packed_words(luminance[second]);
    left = _mm_add_pi16(first_luma, chroma);
    right = _mm_add_pi16(second_luma, chroma);
}
__forceinline __m64 pack_pair(__m64 colours, __m64 bias, __m64 mask, __m64 factors,
                             __m64 green) {
    const __m64 saturated = _mm_adds_pu8(colours, bias);
    const __m64 components = _mm_and_si64(saturated, mask);
    const __m64 product = _mm_madd_pi16(components, factors);
    const __m64 green_bits = _mm_srli_pi32(_mm_and_si64(saturated, green), 11);
    const __m64 joined = _mm_or_si64(product, green_bits);
    return _mm_or_si64(_mm_srli_si64(joined, 16), joined);
}
__forceinline void write_pair(Byte *output, __m64 value) {
    *reinterpret_cast<__m64 *>(output) = value;
    _ReadWriteBarrier();
}
__forceinline void write_pixel_pair(Byte *output, __m64 value) {
    store(output, Dword(_mm_cvtsi64_si32(value)));
}
__forceinline __m64 load_pixel(const Byte *source) {
    const Dword value = load<Dword>(source);
    _ReadWriteBarrier();
    return _mm_cvtsi32_si64(int(value));
}
__forceinline __m64 average_pixel(const Byte *top, const Byte *bottom, __m64 halves) {
    const __m64 first = load_pixel(top);
    const __m64 second = load_pixel(bottom);
    return _mm_add_pi8(_mm_and_si64(_mm_srli_pi32(first, 1), halves),
                         _mm_and_si64(_mm_srli_pi32(second, 1), halves));
}
}

void row_yuy2(const Byte *luma, const Byte *u, const Byte *v, Byte *output, Byte *end) {
    do {
        Dword second_chroma = *v++;
        Dword second_luma = luma[1];
        Dword first_chroma = *u++;
        Dword first_luma = *luma;
        luma += 2;
        store(output,
              first_luma | (first_chroma << 8) | (second_luma << 16) | (second_chroma << 24));
        output += 4;
    } while (output != end);
}

void row_colour(const Byte *luma, const Byte *u, const Byte *v, Byte *output, Byte *end) {
    do {
        Byte first_chroma = *u++, second_chroma = *v++;
        __m64 first, second;
        colour_words(luma, first_chroma, second_chroma, first, second);
        write_pair(output, _mm_packs_pu16(first, second));
        output += 8;
    } while (output != end);
}

void row_packed(const Byte *luma, const Byte *u, const Byte *v, Byte *output, Byte *end) {
    const __m64 bias = packed_words(&dither), mask = packed_words(&component_mask),
                factors = packed_words(&multiplier), green = packed_words(&green_mask);
    do {
        Byte first_chroma = *u++, second_chroma = *v++;
        __m64 first, second;
        colour_words(luma, first_chroma, second_chroma, first, second);
        write_pixel_pair(output, pack_pair(_mm_packs_pu16(first, second), bias, mask, factors,
                                            green));
        output += 4;
    } while (output != end);
}

void row_doubled(const Byte *luma, const Byte *u, const Byte *v, Byte *output, Byte *end) {
    const __m64 bias = packed_words(&dither), mask = packed_words(&component_mask),
                factors = packed_words(&multiplier), green = packed_words(&green_mask);
    do {
        Byte first_chroma = *u++, second_chroma = *v++;
        __m64 first, second;
        colour_words(luma, first_chroma, second_chroma, first, second);
        write_pixel_pair(output, pack_pair(_mm_packs_pu16(first, first), bias, mask, factors,
                                            green));
        write_pixel_pair(output + 4, pack_pair(_mm_packs_pu16(second, second), bias, mask,
                                                factors, green));
        output += 8;
    } while (output != end);
}

void row_horizontal(const Byte *source, Byte *output, Byte *end) {
    const __m64 halves = _mm_set_pi32(2139062143, 2139062143),
                bias = packed_words(&dither), mask = packed_words(&component_mask),
                factors = packed_words(&multiplier), green = packed_words(&green_mask);
    __m64 previous = _mm_and_si64(_mm_srli_pi32(load_pixel(source), 1), halves);
    do {
        const __m64 current = load_pixel(source);
        const __m64 next = _mm_and_si64(_mm_srli_pi32(current, 1), halves);
        source += 4;
        const __m64 pair = _mm_add_pi32(_mm_add_pi8(previous, next),
                                         _mm_slli_si64(current, 32));
        write_pixel_pair(output, pack_pair(pair, bias, mask, factors, green));
        output += 4;
        previous = next;
    } while (output != end);
}

void row_vertical(const Byte *top, const Byte *bottom, Byte *output, Byte *end) {
    const __m64 halves = _mm_set_pi32(2139062143, 2139062143),
                bias = packed_words(&dither), mask = packed_words(&component_mask),
                factors = packed_words(&multiplier), green = packed_words(&green_mask);
    // First iteration starts from the full vertical average, unlike later iterations.
    __m64 previous = average_pixel(top, bottom, halves);
    do {
        const __m64 current = average_pixel(top, bottom, halves);
        const __m64 next = _mm_and_si64(_mm_srli_pi32(current, 1), halves);
        top += 4;
        bottom += 4;
        const __m64 pair = _mm_add_pi32(_mm_add_pi8(previous, next),
                                         _mm_slli_si64(current, 32));
        write_pixel_pair(output, pack_pair(pair, bias, mask, factors, green));
        output += 4;
        previous = next;
    } while (output != end);
}
} // namespace nfs::movies::yuv
