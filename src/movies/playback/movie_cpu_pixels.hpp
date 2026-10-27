#pragma once
#include <cstdint>

namespace nfs::movies::display_pixels {

// User-authorized MAD display compatibility. These integer presentation helpers
// are separate from the native YUV decoder and do not change its arithmetic.
struct AxisSample {
    std::uint32_t first;
    std::uint32_t second;
    std::uint32_t weight;
};

// The caller validates 1 <= source_size <= 32767,
// 1 <= destination_size <= INT_MAX / 4 and position < destination_size.
// Pixel centres use Q8 coordinates; only the source edges are clamped.
constexpr AxisSample sample_axis(std::uint32_t position, std::uint32_t source_size,
                                 std::uint32_t destination_size) noexcept {
    const std::uint64_t centre =
        ((std::uint64_t(position) * 2u + 1u) * source_size * 128u) / destination_size;
    if (centre <= 128u)
        return {0, 0, 0};

    const std::uint64_t coordinate = centre - 128u;
    const std::uint32_t first = std::uint32_t(coordinate >> 8);
    const std::uint32_t last = source_size - 1u;
    if (first >= last)
        return {last, last, 0};
    return {first, first + 1u, std::uint32_t(coordinate & 255u)};
}

// weight is in [0, 256]. Each B/G/R channel is rounded independently to nearest
// with +128 before division by 256; alpha is always opaque.
constexpr std::uint32_t blend(std::uint32_t first, std::uint32_t second,
                              std::uint32_t weight) noexcept {
    const std::uint32_t inverse = 256u - weight;
    // Each 16-bit lane is at most 255*256+128, so B cannot carry into R.

    const std::uint32_t red_blue =
        (((first & 16711935u) * inverse + (second & 16711935u) * weight +
          8388736u) >>
         8) &
        16711935u;
    const std::uint32_t green =
        (((first & 65280u) * inverse + (second & 65280u) * weight +
          32768u) >>
         8) &
        65280u;
    return 4278190080u | red_blue | green;
}

constexpr std::uint16_t pack565(std::uint32_t pixel) noexcept {
    return std::uint16_t(((pixel >> 8) & 63488u) | ((pixel >> 5) & 2016u) |
                         ((pixel >> 3) & 31u));
}

constexpr std::uint16_t pack1555(std::uint32_t pixel) noexcept {
    return std::uint16_t(32768u | ((pixel >> 9) & 31744u) |
                         ((pixel >> 6) & 992u) | ((pixel >> 3) & 31u));
}

constexpr std::uint16_t pack4444(std::uint32_t pixel) noexcept {
    return std::uint16_t(61440u | ((pixel >> 12) & 3840u) |
                         ((pixel >> 8) & 240u) | ((pixel >> 4) & 15u));
}

// A8R3G3B2 is a 16-bit format: opaque alpha occupies the upper byte.
constexpr std::uint16_t pack332(std::uint32_t pixel) noexcept {
    return std::uint16_t(65280u | ((pixel >> 16) & 224u) |
                         ((pixel >> 11) & 28u) | ((pixel >> 6) & 3u));
}

} // namespace nfs::movies::display_pixels
