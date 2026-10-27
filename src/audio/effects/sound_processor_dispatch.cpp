#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend::pending {
using Process = int(__cdecl *)(void *, int, void *, void *, Byte);
extern void filter_samples(void *, int, void *, void *);
extern void effect_samples(void *, int, void *, void *);
extern int resampler_input_count(void *, int);
extern int resampler_output(void *, int, void *, void *);
extern const float converter_bias;

int process_filter(void *processor, int count, void *input, void *output, Byte) {
    if (void *next = field<void *>(processor, 8)) {
        int result = reinterpret_cast<Process>(field<Dword>(next, 0))(next, count, output, input,
                                                                     field<Byte>(processor, 26));
        if (result < 1)
            return result;
    }
    filter_samples(static_cast<Byte *>(processor) + 28, count, input, output);
    return count;
}

int process_effect(void *processor, int count, void *input, void *output, Byte) {
    if (void *next = field<void *>(processor, 8)) {
        count = reinterpret_cast<Process>(field<Dword>(next, 0))(next, count, output, input,
                                                                field<Byte>(processor, 26));
        if (count < 1)
            return count;
    }
    effect_samples(static_cast<Byte *>(processor) + 28, count, input, output);
    return count;
}
// Float bias conversion reads the stored bits, not a numeric cast.
int process_float(void *processor, int count, void *input, void *output, Byte) {
    int result = count;
    if (void *next = field<void *>(processor, 8)) {
        result = reinterpret_cast<Process>(field<Dword>(next, 0))(next, count, output, input,
                                                                 field<Byte>(processor, 26));
        if (result <= 0)
            return result;
    }
    for (int i = 0; i < count; ++i) {
        field<float>(input, i * 4) =
            float(static_cast<long double>(field<float>(input, i * 4)) + converter_bias);
        Dword value = field<Dword>(input, i * 4) & 1048575;
        if (value > 32767 && value < 1015808)
            value = (Dword(-int(value < 524288)) & 65535) - 32768;
        field<std::uint16_t>(output, i * 2) = std::uint16_t(value);
    }
    return count;
}

int stream_samples(int index) {
    if (!control.initialized)
        return -10;
    Dword *stream = stream_nodes[index];
    return int(stream[9] + stream[8]);
}

int process_resampler(void *processor, int count, void *input, void *output, Byte) {
    if (!field<Dword>(processor, 48)) {
        void *next = field<void *>(processor, 8);
        return field<Process>(next, 0)(next, 200, output, input, field<Byte>(processor, 26));
    }
    int required = resampler_input_count(processor, count);
    if (field<int>(processor, 32) >= 0 && stream_samples(field<int>(processor, 32)) < required)
        return 0;
    int result = required;
    if (required > 0) {
        void *next = field<void *>(processor, 8);
        result = field<Process>(next, 0)(next, required, output, input,
                                        field<Byte>(processor, 26));
        if (result <= 0)
            return result;
    }
    int produced = resampler_output(processor, count, input, output);
    if (produced < count)
        zero_memory(static_cast<Byte *>(output) + produced * 4, Dword(count - produced) * 4);
    return count;
}
} // namespace nfs::sound::backend::pending
