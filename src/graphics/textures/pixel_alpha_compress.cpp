#include "nfs/graphics/textures/pixel_compression.hpp"
#include <cfenv>
#include <cstring>

namespace nfs::graphics::surfaces::pixels {
namespace {

constexpr Dword selectors8[8] = {0, 2, 3, 4, 5, 6, 7, 1};
constexpr Dword selectors6[6] = {0, 2, 3, 4, 5, 1};

void diffuse_alpha(float *errors, unsigned pixel, double residual) {
    unsigned x = pixel & 3;
    if (x != 3)
        errors[pixel + 1] = residual * 0.4375 + errors[pixel + 1];
    if (pixel < 12) {
        if (x)
            errors[pixel + 3] = residual * 0.1875 + errors[pixel + 3];
        errors[pixel + 4] = residual * 0.3125 + errors[pixel + 4];
        if (x != 3)
            errors[pixel + 5] = residual * 0.0625 + errors[pixel + 5];
    }
}

} // namespace

// Quantize alpha, fit endpoints, then diffuse selector error.
int encode_dxt5(void *destination, const Colour *pixels) {
    auto *packed = static_cast<std::uint8_t *>(destination);
    float errors[16] = {}, quantized[16];
    float minimum = pixels[0].alpha, maximum = minimum;
    int previous_rounding = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    for (unsigned pixel = 0; pixel < 16; ++pixel) {
        float value = static_cast<double>(errors[pixel]) + pixels[pixel].alpha;
        float biased = static_cast<double>(value) * 255 + 0.5;
        float rounded = static_cast<double>(static_cast<int>(biased)) * 0.0039215688593685626983642578125f;
        quantized[pixel] = rounded;
        if (rounded < minimum)
            minimum = rounded;
        else if (maximum < rounded)
            maximum = rounded;
        diffuse_alpha(errors, pixel, static_cast<double>(value) - rounded);
    }
    std::fesetround(previous_rounding);
    int result = encode_colour_block(packed + 8, pixels, 0);
    if (result < 0)
        return result;
    if (minimum == 1) {
        packed[0] = packed[1] = 255;
        std::memset(packed + 2, 0, 6);
        return 0;
    }

    Dword levels = minimum == 0 || maximum == 1 ? 6 : 8;
    float lower, upper;
    fit_alpha_endpoints(&lower, &upper, quantized, levels);
    std::fesetround(FE_TOWARDZERO);
    float lower_biased = static_cast<double>(lower) * 255 + 0.5;
    float upper_biased = static_cast<double>(upper) * 255 + 0.5;
    Dword lower_byte = static_cast<Dword>(static_cast<int>(lower_biased));
    Dword upper_byte = static_cast<Dword>(static_cast<int>(upper_biased));
    float lower_value = (lower_byte & 255) * 0.0039215688593685626983642578125f;
    float upper_value = (upper_byte & 255) * 0.0039215688593685626983642578125f;
    float palette[8];
    const Dword *selector;
    if (levels == 8) {
        if (static_cast<std::uint8_t>(lower_byte) == static_cast<std::uint8_t>(upper_byte)) {
            packed[0] = static_cast<std::uint8_t>(lower_byte);
            packed[1] = static_cast<std::uint8_t>(upper_byte);
            std::memset(packed + 2, 0, 6);
            std::fesetround(previous_rounding);
            return 0;
        }
        packed[0] = static_cast<std::uint8_t>(upper_byte);
        packed[1] = static_cast<std::uint8_t>(lower_byte);
        palette[0] = upper_value;
        palette[1] = lower_value;
        for (Dword entry = 1; entry < 7; ++entry)
            palette[entry + 1] = (static_cast<double>(entry) * lower_value +
                                  static_cast<double>(7 - entry) * upper_value) *
                                 0.14285714924335479736328125f;
        selector = selectors8;
    } else {
        packed[0] = static_cast<std::uint8_t>(lower_byte);
        packed[1] = static_cast<std::uint8_t>(upper_byte);
        palette[0] = lower_value;
        palette[1] = upper_value;
        for (Dword entry = 1; entry < 5; ++entry)
            palette[entry + 1] = (static_cast<double>(entry) * upper_value +
                                  static_cast<double>(5 - entry) * lower_value) *
                                 0.20000000298023223876953125f;
        palette[6] = 0;
        palette[7] = 1;
        selector = selectors6;
    }
    float last = static_cast<float>(levels - 1);
    float scale = palette[0] != palette[1]
                      ? static_cast<double>(last) / (static_cast<double>(palette[1]) - palette[0])
                      : 0;
    std::memset(errors, 0, sizeof(errors));
    std::fesetround(previous_rounding);
    std::fesetround(FE_TOWARDZERO);
    for (unsigned half = 0; half < 2; ++half) {
        Dword bits = 0;
        for (unsigned pixel = half * 8; pixel < half * 8 + 8; ++pixel) {
            double value = static_cast<double>(errors[pixel]) + pixels[pixel].alpha;
            double position = (value - palette[0]) * scale;
            Dword selected;
            if (position > 0) {
                if (position < last) {
                    float biased = position + 0.5;
                    selected = selector[static_cast<int>(biased)];
                } else if (levels == 6 && (static_cast<double>(palette[1]) + 1) * 0.5 < value)
                    selected = 7;
                else
                    selected = 1;
            } else if (levels != 6 || static_cast<double>(palette[0]) * 0.5 < value)
                selected = 0;
            else
                selected = 6;
            bits = bits >> 3 | selected << 21;
            diffuse_alpha(errors, pixel, value - palette[selected]);
        }
        packed[half * 3 + 2] = static_cast<std::uint8_t>(bits);
        packed[half * 3 + 3] = static_cast<std::uint8_t>(bits >> 8);
        packed[half * 3 + 4] = static_cast<std::uint8_t>(bits >> 16);
    }
    std::fesetround(previous_rounding);
    return 0;
}

} // namespace nfs::graphics::surfaces::pixels
