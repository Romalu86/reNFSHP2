#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {
extern Byte interpolation_state[352];
extern const float scalar_phase_scale, scalar_linear_scale, cubic_three, cubic_five;

// This scalar path reduces the phase to sixteen bits.
void cubic_scalar(Dword count, const void *input, void *output, int *index, Dword *phase,
                           int whole_step, Dword fraction_step) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    auto *samples = static_cast<const float *>(input);
    auto *result = static_cast<float *>(output);
    *phase >>= 16;
    const Dword step = (Dword(whole_step) << 16) | (fraction_step >> 16);
    for (int n = 0; n < int(count); ++n) {
        auto t = static_cast<long double>(*phase) * scalar_phase_scale, t2 = t * t;
        int i = *index;
        auto a = X(samples[i - 1]), b = X(samples[i]), c = X(samples[i + 1]), d = X(samples[i + 2]);
        auto cubic = (((b - c) * cubic_three - a) + d) * 0.5f;
        auto square = (c + c + a) - (b * cubic_five + d) * 0.5f;
        auto value = cubic * (t2 * t) + square * t2;
        value = value + ((c - a) * 0.5f) * t;
        result[n] = float(value + b);
        *phase += step;
        *index = int(Dword(*index) + (*phase >> 16));
        *phase &= 65535;
    }
    *phase <<= 16;
}
// FILD reads the full signed qword scratch.
void linear_scalar(Dword count, const void *input, void *output, int *index, Dword *phase,
                            int whole_step, Dword fraction_step) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    const auto *samples = static_cast<const float *>(input);
    auto *result = static_cast<float *>(output);
    Dword cursor = *phase;
    int i = *index;
    field<Dword>(interpolation_state, 224) = cursor;
    for (Dword n = 0; n < count; ++n) {
        auto fraction =
            static_cast<long double>(field<std::int64_t>(interpolation_state, 224));
        const int before = i;
        Dword next = cursor + fraction_step;
        i = int(Dword(i) + Dword(whole_step) + (next < cursor));
        cursor = next;
        field<Dword>(interpolation_state, 224) = cursor;
        result[n] =
            float((fraction * (X(samples[before + 1]) - samples[before])) * scalar_linear_scale +
                  samples[before]);
    }
    *phase = cursor;
    *index = i;
}
} // namespace nfs::sound::backend::pending
