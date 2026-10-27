#include "nfs/graphics/textures/pixel_blocks.hpp"
#include <cmath>
namespace nfs::graphics::surfaces::pixels {

void unpack_565(Colour *out, Dword value) {
    *out = {float(value >> 11 & 31) * 0.0322580635547637939453125f, float(value >> 5 & 63) * 0.01587301678955554962158203125f,
            float(value & 31) * 0.0322580635547637939453125f, 1};
}
void interpolate(Colour *out, const Colour *a, const Colour *b, float weight) {
    auto *result = reinterpret_cast<float *>(out);
    const auto *first = reinterpret_cast<const float *>(a);
    const auto *second = reinterpret_cast<const float *>(b);
    for (unsigned channel = 0; channel < 4; ++channel)
        result[channel] =
            (static_cast<double>(second[channel]) - first[channel]) * weight + first[channel];
}
// The in-place unpremultiply clamps to one.
int unpremultiply(Colour *block) {
    for (unsigned i = 0; i < 16; ++i) {
        auto &c = block[i];
        if (c.alpha == 0)
            c.red = c.green = c.blue = 0;
        else if (c.alpha < 1) {
            double inverse = 1.0 / static_cast<double>(c.alpha);
            c.red = c.red < c.alpha ? static_cast<float>(inverse * c.red) : 1;
            c.green = c.green < c.alpha ? static_cast<float>(inverse * c.green) : 1;
            c.blue = c.blue < c.alpha ? static_cast<float>(inverse * c.blue) : 1;
        }
    }
    return 0;
}
int premultiply(Colour *output, const Colour *input) {
    for (unsigned i = 0; i < 16; ++i) {
        output[i].red = input[i].alpha * input[i].red;
        output[i].green = input[i].green * input[i].alpha;
        output[i].blue = input[i].blue * input[i].alpha;
        output[i].alpha = input[i].alpha;
    }
    return 0;
}
// The endpoint ordering controls transparent palette entry 3.
int decode_dxt1(Colour *output, const void *packed) {
    const auto *in = static_cast<const std::uint16_t *>(packed);
    Colour palette[4];
    unpack_565(palette, in[0]);
    unpack_565(palette + 1, in[1]);
    if (in[1] < in[0]) {
        interpolate(palette + 2, palette, palette + 1, 0.3333333432674407958984375f);
        interpolate(palette + 3, palette, palette + 1, 0.666666686534881591796875f);
        for (unsigned channel = 1; channel < 4; ++channel) {
            float delta = reinterpret_cast<float *>(palette + 1)[channel] -
                          reinterpret_cast<float *>(palette)[channel];
            reinterpret_cast<float *>(palette + 3)[channel] =
                static_cast<double>(delta) * 0.666666686534881591796875f +
                reinterpret_cast<float *>(palette)[channel];
        }
    } else {
        interpolate(palette + 2, palette, palette + 1, 0.5f);
        palette[3] = {0, 0, 0, 0};
    }
    Dword selectors;
    std::memcpy(&selectors, in + 2, 4);
    for (unsigned i = 0; i < 16; ++i, selectors >>= 2)
        output[i] = palette[selectors & 3];
    return 0;
}

int decode_dxt3(Colour *output, const void *packed) {
    const auto *in = static_cast<const std::uint8_t *>(packed);
    int result = decode_dxt1(output, in + 8);
    if (result < 0)
        return result;
    for (unsigned half = 0; half < 2; ++half) {
        Dword selectors;
        std::memcpy(&selectors, in + half * 4, 4);
        for (unsigned i = 0; i < 8; ++i, selectors >>= 4)
            output[half * 8 + i].alpha = float(selectors & 15) * 0.066666670143604278564453125f;
    }
    return 0;
}

int decode_dxt5(Colour *output, const void *packed) {
    const auto *in = static_cast<const std::uint8_t *>(packed);
    int result = decode_dxt1(output, in + 8);
    if (result < 0)
        return result;
    float alpha[8] = {in[0] * 0.0039215688593685626983642578125f, in[1] * 0.0039215688593685626983642578125f};
    if (in[1] < in[0]) {
        for (unsigned i = 1; i < 7; ++i)
            alpha[i + 1] =
                (static_cast<double>(i) * alpha[1] + static_cast<double>(7 - i) * alpha[0]) *
                0.14285714924335479736328125f;
    } else {
        for (unsigned i = 1; i < 5; ++i)
            alpha[i + 1] =
                (static_cast<double>(i) * alpha[1] + static_cast<double>(5 - i) * alpha[0]) *
                0.20000000298023223876953125f;
        alpha[6] = 0;
        alpha[7] = 1;
    }
    for (unsigned half = 0; half < 2; ++half) {
        Dword selectors =
            in[2 + half * 3] | Dword(in[3 + half * 3]) << 8 | Dword(in[4 + half * 3]) << 16;
        for (unsigned i = 0; i < 8; ++i, selectors >>= 3)
            output[half * 8 + i].alpha = alpha[selectors & 7];
    }
    return 0;
}
// (vtable callback entrypoints).
int decode_dxt2(Colour *out, const void *in) {
    int result = decode_dxt3(out, in);
    if (result < 0)
        return result;
    return unpremultiply(out);
}
int decode_dxt4(Colour *out, const void *in) {
    int result = decode_dxt5(out, in);
    if (result < 0)
        return result;
    return unpremultiply(out);
}
} // namespace nfs::graphics::surfaces::pixels
