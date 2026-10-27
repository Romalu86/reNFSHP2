#include "nfs/audio/codecs/sound_codecs.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::codecs {

void decode_predictor_scalar(void *data) {
    auto *record = static_cast<Dword *>(data);
    auto *out = reinterpret_cast<float *>(record[4]);
    field<Dword>(out, -4) = record[1];
    field<Dword>(out, -8) = record[2];
    auto *input = reinterpret_cast<const Byte *>(record[3]);

    auto X = [&](float v) { return static_cast<long double>(v); };
    while (int(record[0]) > 0) {
        record[0] -= 28;
        unsigned filter = *input >> 4, scale = (*input & 15) * 64;
        const float a = table<float>(7130376 + filter * 4),
                    b = table<float>(7130392 + filter * 4);
        for (unsigned j = 0; j < 14; ++j) {
            Byte v = input[1 + j];
            float upper = table<float>(7130408 + scale + (v >> 4) * 4),
                  lower = table<float>(7130408 + scale + (v & 15) * 4);
            out[0] = float((X(out[-1]) * a + upper) + X(out[-2]) * b);
            out[1] = float((X(out[0]) * a + lower) + X(out[-1]) * b);
            out += 2;
        }
        input += 15;
    }
    field<float>(record, 4) = out[-1];
    field<float>(record, 8) = out[-2];
    record[4] = reinterpret_cast<Dword>(out);
}
}
