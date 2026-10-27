#include "nfs/graphics/textures/pixel_compression.hpp"
#include <cfenv>
namespace nfs::graphics::surfaces::pixels {
// The endpoint fitter is an explicit unresolved dependency.
int encode_dxt1(void *out, const Colour *in) {
    int result = encode_colour_block(out, in, 1);
    return result < 0 ? result : 0;
}
// Alpha error diffusion precedes the colour-block fitter.
int encode_dxt3(void *packed, const Colour *input) {
    auto *out = static_cast<Dword *>(packed);
    out[0] = out[1] = 0;
    float error[16] = {};
    int rounding = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    for (Dword i = 0; i < 16; ++i) {
        float value = static_cast<double>(error[i]) + input[i].alpha;
        float quantized = static_cast<double>(value) * 15 + 0.5;
        Dword alpha = static_cast<Dword>(static_cast<int>(quantized));
        out[i >> 3] = (out[i >> 3] >> 4) | (alpha << 28);
        double difference =
            static_cast<double>(value) - static_cast<double>(alpha) * 0.066666670143604278564453125f;
        Dword x = i & 3;
        if (x != 3)
            error[i + 1] = difference * 0.4375 + error[i + 1];
        if (i < 12) {
            if (x)
                error[i + 3] = difference * 0.1875 + error[i + 3];
            error[i + 4] = difference * 0.3125 + error[i + 4];
            if (x != 3)
                error[i + 5] = difference * 0.0625 + error[i + 5];
        }
    }
    std::fesetround(rounding);
    return encode_colour_block(out + 2, input, 0);
}

int encode_dxt2(void *out, const Colour *in) {
    Colour scratch[16];
    int result = premultiply(scratch, in);
    if (result < 0)
        return result;
    result = encode_dxt3(out, scratch);
    return result < 0 ? result : 0;
}
int encode_dxt4(void *out, const Colour *in) {
    Colour scratch[16];
    int result = premultiply(scratch, in);
    if (result < 0)
        return result;
    result = encode_dxt5(out, scratch);
    return result < 0 ? result : 0;
}
} // namespace nfs::graphics::surfaces::pixels
