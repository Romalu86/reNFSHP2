#include "nfs/graphics/textures/pixel_compression.hpp"
#include <cfenv>
#include <cstring>

namespace nfs::graphics::surfaces::pixels {
namespace {

constexpr float colour_weights[3] = {0.297036647796630859375f, 1, 0.1007827818393707275390625f};
constexpr float inverse_weights[3] = {3.366588115692138671875f, 1, 9.92232990264892578125f};
constexpr Dword selectors4[4] = {0, 2, 3, 1};
constexpr Dword selectors3[3] = {0, 2, 1};

void diffuse_colour(Colour *errors, unsigned pixel, double red, float green, float blue) {
    auto update = [&](unsigned target, double weight) {
        errors[target].red = red * weight + errors[target].red;
        errors[target].green = static_cast<double>(green) * weight + errors[target].green;
        errors[target].blue = static_cast<double>(blue) * weight + errors[target].blue;
    };
    unsigned x = pixel & 3;
    if (x != 3)
        update(pixel + 1, 0.4375);
    if (pixel < 12) {
        if (x)
            update(pixel + 3, 0.1875);
        update(pixel + 4, 0.3125);
        if (x != 3)
            update(pixel + 5, 0.0625);
    }
}

} // namespace

// Error diffusion, weighted endpoint fitting and block selectors.
int encode_colour_block(void *destination, const Colour *pixels, Dword allow_transparency) {
    auto *packed = static_cast<std::uint16_t *>(destination);
    Dword levels = 4;
    if (allow_transparency) {
        unsigned transparent = 0;
        for (unsigned pixel = 0; pixel < 16; ++pixel)
            if (pixels[pixel].alpha < 0.5f)
                ++transparent;
        if (transparent == 16) {
            packed[0] = 0;
            packed[1] = 65535;
            Dword indices = 4294967295u;
            std::memcpy(packed + 2, &indices, 4);
            return 0;
        }
        levels = transparent ? 3 : 4;
    }

    Colour quantized[16];
    Colour errors[16] = {};
    int previous_rounding = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    for (unsigned pixel = 0; pixel < 16; ++pixel) {
        float red = static_cast<double>(errors[pixel].red) + pixels[pixel].red;
        double green_sum = static_cast<double>(pixels[pixel].green) + errors[pixel].green;
        float green = green_sum;
        float blue = static_cast<double>(errors[pixel].blue) + pixels[pixel].blue;
        float red_biased = static_cast<double>(red) * 31 + 0.5;
        float green_biased = green_sum * 63 + 0.5;
        float blue_biased = static_cast<double>(blue) * 31 + 0.5;
        float red_quantized = static_cast<double>(static_cast<int>(red_biased)) * 0.0322580635547637939453125f;
        float green_quantized =
            static_cast<double>(static_cast<int>(green_biased)) * 0.01587301678955554962158203125f;
        float blue_quantized = static_cast<double>(static_cast<int>(blue_biased)) * 0.0322580635547637939453125f;
        diffuse_colour(errors, pixel, static_cast<double>(red) - red_quantized,
                       static_cast<double>(green) - green_quantized,
                       static_cast<double>(blue) - blue_quantized);
        quantized[pixel] = {red_quantized * colour_weights[0], green_quantized * colour_weights[1],
                            blue_quantized * colour_weights[2], 1};
    }
    std::fesetround(previous_rounding);

    float lower[3], upper[3];
    fit_colour_endpoints(lower, upper, quantized, levels);
    float lower_rgb[3], upper_rgb[3];
    for (unsigned channel = 0; channel < 3; ++channel) {
        lower_rgb[channel] = lower[channel] * inverse_weights[channel];
        upper_rgb[channel] = upper[channel] * inverse_weights[channel];
    }
    std::uint16_t lower_packed = static_cast<std::uint16_t>(pack_565(lower_rgb));
    std::uint16_t upper_packed = static_cast<std::uint16_t>(pack_565(upper_rgb));
    if (levels == 4 && lower_packed == upper_packed) {
        packed[0] = lower_packed;
        packed[1] = upper_packed;
        Dword indices = 0;
        std::memcpy(packed + 2, &indices, 4);
        return 0;
    }

    Colour endpoints[2];
    unpack_565(endpoints, lower_packed);
    unpack_565(endpoints + 1, upper_packed);
    for (unsigned endpoint = 0; endpoint < 2; ++endpoint) {
        auto *components = reinterpret_cast<float *>(endpoints + endpoint);
        for (unsigned channel = 0; channel < 3; ++channel)
            components[channel] *= colour_weights[channel];
    }
    Colour palette[4];
    if ((levels == 3) == (lower_packed <= upper_packed)) {
        packed[0] = lower_packed;
        packed[1] = upper_packed;
        palette[0] = endpoints[0];
        palette[1] = endpoints[1];
    } else {
        packed[0] = upper_packed;
        packed[1] = lower_packed;
        palette[0] = endpoints[1];
        palette[1] = endpoints[0];
    }

    float red_delta = static_cast<double>(palette[1].red) - palette[0].red;
    const float *first = reinterpret_cast<const float *>(palette);
    const float *second = reinterpret_cast<const float *>(palette + 1);
    auto *third = reinterpret_cast<float *>(palette + 2);
    auto *fourth = reinterpret_cast<float *>(palette + 3);
    float factor = levels == 3 ? 0.5f : 0.3333333432674407958984375f;
    for (unsigned channel = 0; channel < 4; ++channel) {
        double delta =
            channel == 0 ? red_delta : static_cast<double>(second[channel]) - first[channel];
        third[channel] = delta * factor;
        if (levels == 4)
            fourth[channel] = delta * 0.666666686534881591796875f + first[channel];
        third[channel] = static_cast<double>(third[channel]) + first[channel];
    }

    float blue_delta = static_cast<double>(palette[1].blue) - palette[0].blue;
    double green_delta = static_cast<double>(palette[1].green) - palette[0].green;
    float last = static_cast<float>(levels - 1);
    double scale = lower_packed == upper_packed
                       ? 0
                       : static_cast<double>(last) / (static_cast<double>(red_delta) * red_delta +
                                                      green_delta * green_delta +
                                                      static_cast<double>(blue_delta) * blue_delta);
    float projection[3] = {static_cast<double>(red_delta) * scale, scale * green_delta,
                           scale * blue_delta};
    std::memset(errors, 0, sizeof(errors));
    const Dword *selector = levels == 3 ? selectors3 : selectors4;
    Dword indices = 0;
    std::fesetround(FE_TOWARDZERO);
    for (unsigned pixel = 0; pixel < 16; ++pixel) {
        if (levels == 3 && pixels[pixel].alpha < 0.5f) {
            indices = indices >> 2 | 3221225472u;
            continue;
        }
        float red = static_cast<double>(colour_weights[0]) * pixels[pixel].red + errors[pixel].red;
        float green =
            static_cast<double>(colour_weights[1]) * pixels[pixel].green + errors[pixel].green;
        float blue =
            static_cast<double>(colour_weights[2]) * pixels[pixel].blue + errors[pixel].blue;
        double position = (static_cast<double>(green) - palette[0].green) * projection[1] +
                          (static_cast<double>(blue) - palette[0].blue) * projection[2] +
                          (static_cast<double>(red) - palette[0].red) * projection[0];
        Dword selected = 0;
        if (position > 0) {
            if (position < last) {
                float biased = position + 0.5;
                selected = selector[static_cast<int>(biased)];
            } else
                selected = 1;
        }
        indices = indices >> 2 | selected << 30;
        diffuse_colour(
            errors, pixel,
            (static_cast<double>(red) - palette[selected].red) * quantized[pixel].alpha,
            (static_cast<double>(green) - palette[selected].green) * quantized[pixel].alpha,
            (static_cast<double>(blue) - palette[selected].blue) * quantized[pixel].alpha);
    }
    std::fesetround(previous_rounding);
    std::memcpy(packed + 2, &indices, 4);
    return 0;
}

} // namespace nfs::graphics::surfaces::pixels
