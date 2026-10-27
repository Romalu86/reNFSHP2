#include "nfs/audio/music/music_clients.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void construct_waveform(void *node, Dword kind, int channel) {
    sound::field<Dword>(node, 28) = kind;
    sound::field<int>(node, 32) = channel;
    sound::field<Dword>(node, 36) = 0;
    for (unsigned row = 0; row < 2; ++row)
        if (sound::field<Dword>(wave_processors, row * 32) == kind)
            sound::field<void *>(wave_processors, row * 32 + 8 + channel * 4) = node;
}

void destroy_waveform(void *node) {
    for (unsigned row = 0; row < 2; ++row)
        if (sound::field<Dword>(wave_processors, row * 32) == sound::field<Dword>(node, 28))
            sound::field<Dword>(wave_processors, row * 32 + 8 + sound::field<Dword>(node, 32) * 4) =
                0;
}
// Callers provide a fifth cdecl argument, which is unused.
int process_waveform(void *node, int samples, void *input, void *output, Byte) {
    if (void *next = sound::field<void *>(node, 8)) {
        using Process = int(__cdecl *)(void *, int, void *, void *, Byte);
        int result = sound::field<Process>(next, 0)(next, samples, output, input,
                                                    sound::field<Byte>(node, 26));
        if (result < 1)
            return result;
    }
    auto *source = static_cast<Dword *>(input), *destination = static_cast<Dword *>(output);
    auto &buffer_selector = static_cast<volatile Dword &>(waveform_buffer);
    for (int index = 0; index < samples; ++index) {
        const bool alternate = buffer_selector != 0;
        Dword value = source[index];
        if (!alternate) {
            destination[index] = value;
            state::spectrum<Dword>(181264 + Dword(index) * 4) = source[index];
        } else {
            state::spectrum<Dword>(214032 + Dword(index) * 4) = value;
            destination[index] = value;
        }
    }
    const Dword cursor = static_cast<volatile Dword &>(state::spectrum<Dword>(audio_layout::spectrum_input_cursor));
    if (samples > 0) {
        for (Dword index = 0; index < (Dword(samples) & 1073741823); ++index)
            state::spectrum<Dword>(audio_layout::spectrum_imaginary_input + (cursor + index) * 4) = 0;
        for (Dword index = 0; index < (Dword(samples) & 1073741823); ++index)
            state::spectrum<Dword>(audio_layout::spectrum_real_input + (cursor + index) * 4) = source[index];
    }
    const Dword next_cursor = cursor + Dword(samples);
    state::spectrum<Dword>(audio_layout::spectrum_input_cursor) = next_cursor;
    if (int(next_cursor) >= 512) {
        spectrum::transform(512, 1, &state::spectrum<float>(audio_layout::spectrum_real_input),
                            &state::spectrum<float>(audio_layout::spectrum_imaginary_input), &state::spectrum<float>(audio_layout::spectrum_real_output),
                            &state::spectrum<float>(audio_layout::spectrum_imaginary_output));
        state::spectrum<Dword>(audio_layout::spectrum_input_cursor) = 0;
    }
    buffer_selector = buffer_selector == 0;
    return samples;
}
} // namespace nfs::game::audio
