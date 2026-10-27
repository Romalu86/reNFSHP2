#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend {
extern const float mix_effect_scale;

void mixer_set_pan(void *context, Dword pan, Dword) {
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i) {
        const Dword value =
            field<Byte>(context, 0) == 1 ? pan : field<std::uint16_t>(context, 16 + i * 2);
        fill_pan(value, backends + field<std::int16_t>(context, 4 + i * 2));
    }
    submit_volume(field<std::int16_t>(context, 4));
}
// The incoming value is unused; saved per-voice volume/effect fields drive it.
void mixer_set_effect(void *context, int) {
    Voice *voice = control.voices + field<std::int16_t>(context, 4);
    const float gain = float(static_cast<long double>(field<std::int16_t>(voice, 86)) *
                             field<std::int8_t>(voice, 69) * mix_effect_scale);
    for (unsigned i = 0; i < field<Byte>(voice, 31); ++i)
        set_mix_effect(field<std::int16_t>(voice, 4 + i * 2), 0, gain);
}

void mixer_set_rate(void *context, int rate) {
    const long double scaled = static_cast<long double>(rate) * field<float>(context, 28);
    const Dword value = Dword(game::runtime::hardware::truncate_integer_native80(&scaled));
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i)
        pending::set_processor_rate(field<std::int16_t>(context, 4 + i * 2), value);
}

void mixer_set_voice_value(void *context, Dword value) {
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i)
        pending::set_processor_seek(field<std::int16_t>(context, 4 + i * 2), value);
}

void mixer_seek(void *context, Dword milliseconds) {
    const auto rate = field<std::uint16_t>(&nfs::sound::pending::bank_limits, 2);
    const float value = float(static_cast<long double>(int(milliseconds)) /
                              (static_cast<long double>(rate) * 0.5f));
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i)
        pending::set_processor_loop(field<std::int16_t>(context, 4 + i * 2), value);
}

void mixer_set_loop(void *context, Dword value) {
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i)
        pending::set_processor_effect(field<std::int16_t>(context, 4 + i * 2), value);
}
// Tail forwards the first channel index.
int mixer_position(void *context) {
    return pending::processor_position(field<std::int16_t>(context, 4));
}

void mixer_stop(void *context) {
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i) {
        destroy_mix_voice(field<std::int16_t>(context, 4 + i * 2));
        retire_voice(field<std::int16_t>(context, 4 + i * 2));
    }
}

int mixer_start(void *context) {
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i)
        activate_mix_voice(field<std::int16_t>(context, 4 + i * 2));
    return 0;
}

int mixer_create(void *context, const void *parameters) {
    field<Byte>(context, 0) = field<Byte>(parameters, 4);
    const auto rate = field<std::uint16_t>(&nfs::sound::pending::bank_limits, 2);
    field<float>(context, 28) =
        float(static_cast<long double>(field<std::uint16_t>(parameters, 0)) * 16 / rate);
    for (unsigned i = 0; i < field<Byte>(context, 0); ++i) {
        field<std::uint16_t>(context, 16 + i * 2) =
            field<std::uint16_t>(parameters, 76 + i * 2);
        int index =
            field<std::int16_t>(control.voices + field<std::int16_t>(parameters, 2), 4 + i * 2);
        field<std::int16_t>(context, 4 + i * 2) = std::int16_t(index);
        start_mix_voice(index, field<Byte>(parameters, 5), field<Dword>(parameters, 20) != 0,
                        field<Dword>(parameters, 24 + i * 4), field<Dword>(parameters, 48),
                        field<int>(parameters, 52 + i * 4), field<Byte>(parameters, 4),
                        field<Dword>(parameters, 20), field<Dword>(parameters, 12),
                        field<int>(parameters, 16), 0, 0, Byte(i));
    }
    return 1;
}
} // namespace nfs::sound::backend
