#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {
// The feedback accumulator stays extended until its final store.
void filter_samples(void *state, int count, void *input, void *output) {

    auto X = [&](float value) { return static_cast<long double>(value); };
    auto previous = X(field<float>(state, 0));
    int index = 0;
    do {
        previous = X(field<float>(state, 8)) * field<float>(input, index * 4) +
                   previous * field<float>(state, 4);
        field<float>(output, index * 4) = float(previous);
        ++index;
    } while (index < count);
    field<float>(state, 0) = float(previous);
}
// Output stores precede history reloads, including aliased arrays.
void effect_samples(void *state, int count, void *input, void *output) {

    auto X = [&](float value) { return static_cast<long double>(value); };
    auto *s = static_cast<float *>(state);
    auto *in = static_cast<float *>(input);
    auto *out = static_cast<float *>(output);
    for (int i = 0; i < count; ++i) {
        out[i] = float((X(s[7]) + in[i]) * s[8] + (X(s[2]) + s[4]) * s[11] +
                       (X(s[5]) + s[1]) * s[10] + (X(s[0]) + s[6]) * s[9] + X(s[3]) * s[12]);
        for (unsigned history = 7; history > 0; --history)
            s[history] = s[history - 1];
        s[0] = in[i];
    }
}
} // namespace nfs::sound::backend::pending
