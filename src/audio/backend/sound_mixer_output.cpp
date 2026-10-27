#include "nfs/audio/effects/sound_reverb.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend {
extern const float fade_weights[16];
using Mix = void(__cdecl *)(int, float, const float *, float *);

void fade_tail(float *previous, float *output) {

    for (unsigned i = 0; i < 16; ++i)
        output[i] = float(static_cast<long double>(*previous) * fade_weights[i] + output[i]);
    *previous = 0;
}

void ramp_mix(float first, float last, const float *input, float *output) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    auto step = (X(last) - first) * fade_weights[15];
    for (unsigned i = 0; i < 16; ++i) {
        auto gain = i == 0   ? X(first) + step
                    : i == 1 ? (step + step) + first
                             : (X(float(i + 1)) * step) + first;
        output[i] = float(gain * input[i] + output[i]);
    }
}

int prime_mix_voice(void *voice) {
    void *node = field<void *>(voice, 48);
    int result = field<pending::Processor>(node, 0)(node, 16, mixer<void *>(8717992),
                                                    mixer<void *>(8717996), 0);
    auto submit = [&](unsigned current, unsigned target, float *output) {
        float a = field<float>(voice, current), b = field<float>(voice, target);
        if (a != b) {
            ramp_mix(a, b, mixer<const float *>(8717996), output);
            field<float>(voice, current) = b;
        } else if (b != 0)
            mixer<Mix>(8718204)(16, b, mixer<const float *>(8717996), output);
    };
    if (result > 0) {
        field<Dword>(voice, 44) = field<Dword>(mixer<const float *>(8717996), 60);
        if (mixer<Byte>(8717711))
            submit(36, 40, reinterpret_cast<float *>(effects::storage + 512));
        for (unsigned c = 0; c < pending::mix_modes; ++c)
            submit(4 + c * 4, 20 + c * 4, mixer<float *>(8718016 + c * 4));
        return 16;
    }
    field<Dword>(voice, 36) = field<Dword>(voice, 40);
    for (unsigned c = 0; c < pending::mix_modes; ++c)
        field<Dword>(voice, 4 + c * 4) = field<Dword>(voice, 20 + c * 4);
    return 0;
}

void retire_mix_voice(int index) {
    destroy_mix_voice(index);
    mixer<void(__cdecl *)(int)>(8717704)(index);
}

void mix_channels(void *destinations, int count) {
    if (mixer<Byte>(8717711) && mixer<float>(8718052) != 0)
        fade_tail(&mixer<float>(8718052), reinterpret_cast<float *>(effects::storage + 512));
    for (unsigned c = 0; c < pending::mix_modes; ++c) {
        zero_memory(mixer<void *>(8718016 + c * 4), Dword(count) * 4);
        if (mixer<float>(8718036 + c * 4) != 0)
            fade_tail(&mixer<float>(8718036 + c * 4), mixer<float *>(8718016 + c * 4));
    }
    for (unsigned i = 0; i < mixer<Byte>(8717700); ++i) {
        auto *voice = mix_voices + i;
        if (field<Byte>(voice, 0) != 2)
            continue;
        int prefix = 0, remaining = count;
        if (field<Byte>(voice, 1)) {
            prefix = prime_mix_voice(voice);
            field<Byte>(voice, 1) = 0;
            remaining = count - prefix;
        }
        if (remaining) {
            void *node = field<void *>(voice, 48);
            int n = field<pending::Processor>(node, 0)(node, remaining, mixer<void *>(8717992),
                                                       mixer<void *>(8717996), 0);
            if (n < 0)
                retire_mix_voice(int(i));
            else if (n) {
                field<Dword>(voice, 44) =
                    field<Dword>(mixer<const float *>(8717996), (n - 1) * 4);
                for (unsigned c = 0; c < pending::mix_modes; ++c)
                    if (field<float>(voice, 20 + c * 4) != 0)
                        mixer<Mix>(8718204)(n, field<float>(voice, 20 + c * 4),
                                             mixer<const float *>(8717996),
                                             mixer<float *>(8718016 + c * 4) + prefix);
                if (mixer<Byte>(8717711) && field<float>(voice, 40) != 0) {
                    mixer<Mix>(8718204)(n, field<float>(voice, 40),
                                         mixer<const float *>(8717996),
                                         reinterpret_cast<float *>(effects::storage + 512) +
                                             prefix);
                    effects::at<Dword>(8721808) = 0;
                }
            }
        }
    }
    if (auto effect = mixer<void(__cdecl *)(int)>(8717716))
        effect(count);
    for (unsigned c = 0; c < pending::mix_modes; ++c) {
        void *input = mixer<void *>(8718016 + c * 4),
             *filter = mixer<void *>(8718072 + c * 4);
        if (filter) {
            field<pending::Processor>(filter, 0)(filter, count, input, mixer<void *>(8717992), 0);
            input = mixer<void *>(8717992);
        }
        void *converter = mixer<void *>(8718056 + c * 4);
        void *output = field<void *>(destinations, c * 4);
        field<pending::Processor>(converter, 0)(converter, count, input, output, 0);
    }
}

void interleave_stereo(int count, const void *input, void *output) {
    int cursor = count;
    do {
        Dword left = field<Dword>(input, cursor * 2 - 4),
             right = field<Dword>(input, count * 2 + cursor * 2 - 4);
        field<Dword>(output, cursor * 4 - 8) = (left << 16) | (right & 65535);
        field<Dword>(output, cursor * 4 - 4) = (left & 4294901760u) | (right >> 16);
        int before = cursor;
        cursor -= 2;
        if (!cursor || before <= 1)
            break;
    } while (true);
}

void render_mixer(void *destinations, int count) {
    Dword outputs[4];
    for (unsigned c = 0; c < pending::mix_modes; ++c)
        outputs[c] = field<Dword>(destinations, c * 4);
    if (!mixer<Byte>(8717702)) {
        while (count > 0) {
            int n = count < 257 ? count : 256;
            mix_channels(outputs, n);
            for (unsigned c = 0; c < pending::mix_modes; ++c)
                outputs[c] += mixer<std::uint16_t>(8717708);
            count -= 256;
        }
    } else {
        Dword temporary[2];
        temporary[0] = mixer<Dword>(8718032);
        Dword destination = outputs[0];
        while (count > 0) {
            int n = count < 257 ? count : 256;
            temporary[1] = mixer<Dword>(8718032) + Dword(n) * 2;
            mix_channels(temporary, n);
            interleave_stereo(n, mixer<void *>(8718032), reinterpret_cast<void *>(destination));
            count -= 256;
            destination += Dword(n) << (pending::mix_modes & 31);
        }
    }
}
} // namespace nfs::sound::backend
