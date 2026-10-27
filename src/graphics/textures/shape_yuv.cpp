#include "nfs/graphics/textures/shape_conversion.hpp"
#include <cstring>

namespace nfs::graphics::textures::shape::rows {
namespace {

Byte truncate_channel(double value) {
    if (value > 255)
        value = 255;
    if (value < 0)
        value = 0;
    return static_cast<Byte>(static_cast<int>(value));
}

// Full-range SHAPE YUY2 differs from the D3DX codec.
void decode_yuy2_pair(Byte *output, const Byte *input) {
    double cb = int(input[1]) - 128, cr = int(input[3]) - 128;
    float red = cr * 1.13999998569488525390625f;
    float green = cr * -0.58099997043609619140625f + cb * -0.393999993801116943359375f;
    float blue = cr * 0.0f + cb * 2.0280001163482666015625f;
    for (unsigned pixel = 0; pixel < 2; ++pixel) {
        double luminance = input[pixel * 2] * 1.0625f;
        output[pixel * 4 + 3] = 255;
        output[pixel * 4 + 2] = truncate_channel(luminance + red);
        output[pixel * 4 + 1] = truncate_channel(luminance + green);
        output[pixel * 4] = truncate_channel(luminance + blue);
    }
}

// Luma is truncated before the integer 224/255 range conversion.
void encode_yuy2_pair(Byte *output, const Byte *input) {
    float mean_red = (double(input[2]) + input[6]) * 0.5;
    float mean_blue = (double(input[0]) + input[4]) * 0.5;
    float mean_green = (double(input[1]) + input[5]) * 0.5;
    float mean_luminance = static_cast<double>(mean_green) * 0.58700001239776611328125f +
                           static_cast<double>(mean_blue) * 0.114000000059604644775390625f +
                           static_cast<double>(mean_red) * 0.2989999949932098388671875f;
    float first_luminance = double(input[2]) * 0.2989999949932098388671875f + double(input[0]) * 0.114000000059604644775390625f +
                            double(input[1]) * 0.58700001239776611328125f;
    double second_luminance = double(input[6]) * 0.2989999949932098388671875f +
                              double(input[4]) * 0.114000000059604644775390625f + double(input[5]) * 0.58700001239776611328125f;
    float cb = (static_cast<double>(mean_blue) - mean_luminance) * 0.4930000007152557373046875f + 128;
    float cr = (static_cast<double>(mean_red) - mean_luminance) * 0.87699997425079345703125f + 128;
    output[0] = static_cast<Byte>(Dword(truncate_channel(first_luminance)) * 224 / 255 + 16);
    output[1] = truncate_channel(cb);
    output[2] = static_cast<Byte>(Dword(truncate_channel(second_luminance)) * 224 / 255 + 16);
    output[3] = truncate_channel(cr);
}

} // namespace

// Preserve both half-pair alignment and the odd trailing pixel.
void yuy2_to_bgra(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    if ((reinterpret_cast<Dword>(input) & 2) && count != 0) {
        Byte pair[8];
        decode_yuy2_pair(pair, input - 2);
        std::memcpy(output, pair + 4, 4);
        input += 2;
        output += 4;
        --count;
    }
    for (int pair = 0; pair < count / 2; ++pair) {
        decode_yuy2_pair(output, input);
        input += 4;
        output += 8;
    }
    if (count > 0 && (count & 1)) {
        Byte pair[8];
        decode_yuy2_pair(pair, input);
        std::memcpy(output, pair, 4);
    }
}

// A partial pair updates shared chroma and only its own luma.
void bgra_to_yuy2(void *destination, const void *source, int count) {
    auto *output = static_cast<Byte *>(destination);
    const auto *input = static_cast<const Byte *>(source);
    if (reinterpret_cast<Dword>(output) & 2) {
        Byte duplicate[8], pair[4];
        std::memcpy(duplicate, input, 4);
        std::memcpy(duplicate + 4, input, 4);
        encode_yuy2_pair(pair, duplicate);
        output[0] = pair[2];
        output[-1] = pair[1];
        output[1] = pair[3];
        output += 2;
        input += 4;
        --count;
    }
    for (int pair = 0; pair < count / 2; ++pair) {
        encode_yuy2_pair(output, input);
        output += 4;
        input += 8;
    }
    if (count & 1) {
        Byte duplicate[8], pair[4];
        std::memcpy(duplicate, input, 4);
        std::memcpy(duplicate + 4, input, 4);
        encode_yuy2_pair(pair, duplicate);
        output[0] = pair[2];
        output[1] = pair[1];
        output[3] = pair[3];
    }
}

} // namespace nfs::graphics::textures::shape::rows
