#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::pending {
// Keep the real mixer/DirectSound paths and callback ownership.
int start_backend(int index, Dword length, Dword loop, Dword, Dword, Dword seek, Dword flags,
                           const Dword *sources, Dword loop_data, const Dword *effects, Dword stream) {
    using namespace backend;
    auto *state = backends + index;
    auto *voice = control.voices + index;
    void *context = reinterpret_cast<Byte *>(state) + 28;
    field<Dword>(state, 256) = 0;
    for (unsigned offset = 172; offset <= 208; offset += 4)
        field<void *>(state, offset) = context;
    const auto format = field<std::uint16_t>(voice, 32);
    auto callback = [&](unsigned offset, auto function) {
        field<Dword>(state, offset) = reinterpret_cast<Dword>(function);
    };
    alignas(4) Byte parameters[84];
    if (format & 16) {
        if (field<Byte>(voice, 31) > 1)
            return -7;
        void *buffer = nullptr;
        if (!(format & 1)) {
            field<std::uint16_t>(parameters, 0) = field<std::uint16_t>(voice, 28);
            field<std::uint16_t>(parameters, 2) = std::uint16_t(index);
            field<std::uint16_t>(parameters, 4) = field<std::uint16_t>(voice, 30);
            field<Dword>(parameters, 8) = length;
            field<Dword>(parameters, 12) = loop;
            field<Dword>(parameters, 16) = field<Dword>(voice, 16);
            field<Dword>(parameters, 20) = sources[0];
            field<Dword>(parameters, 24) = effects[0];
            void *chain = reinterpret_cast<Byte *>(state) + 100;
            construct_buffer_chain(chain, parameters);
            for (unsigned offset = 188; offset <= 200; offset += 4)
                field<void *>(state, offset) = chain;
            callback(236, chain_set_loop);
            callback(240, chain_set_effect);
            callback(224, chain_set_value);
            callback(252, chain_remaining);
        } else {
            if (!stream)
                return -15;
            void *binding = find_stream(int(stream));
            if (!binding)
                return -9;
            buffer = reinterpret_cast<void *>(stream_devices[field<Dword>(binding, 4) * 6]);
            field<Dword>(state, 236) = field<Dword>(state, 240) = field<Dword>(state, 224) = 0;
            callback(252, device_position);
        }
        field<void *>(parameters, 0) = direct_sound;
        field<void *>(parameters, 4) = buffer;
        field<std::uint16_t>(parameters, 8) = field<std::uint16_t>(voice, 28);
        field<Byte>(parameters, 10) = Byte(int(loop) > 0);
        field<Byte>(parameters, 11) = use_effect_interface;
        if (!device_create(context, parameters)) {
            if (field<Byte>(voice, 32) & 2)
                clear_backend_chain(reinterpret_cast<Byte *>(state) + 100);
            return -9;
        }
        callback(220, device_set_rate);
        callback(232, device_set_pan);
        callback(244, device_set_buffer_ms);
        callback(248, device_remaining);
        callback(216, device_destroy);
        callback(228, device_set_effect);
        if (field<Byte>(voice, 32) & 2) {
            field<std::uint16_t>(state, 52) = 0;
            insert_processor(reinterpret_cast<void **>(reinterpret_cast<Byte *>(state) + 212),
                             context);
        }
    } else if (format & 4) {
        field<std::uint16_t>(parameters, 0) = field<std::uint16_t>(voice, 28);
        field<std::uint16_t>(parameters, 2) = std::uint16_t(index);
        field<Byte>(parameters, 4) = field<Byte>(voice, 31);
        field<Byte>(parameters, 5) = field<Byte>(voice, 30);
        field<Dword>(parameters, 12) = length;
        field<Dword>(parameters, 16) = loop;
        field<Dword>(parameters, 20) = field<Dword>(voice, 16);
        field<Dword>(parameters, 48) = loop_data;
        for (int channel = 0; channel < field<Byte>(voice, 31); ++channel) {
            field<Dword>(parameters, 24 + channel * 4) = sources[channel];
            field<Dword>(parameters, 52 + channel * 4) = effects[channel];
            field<std::uint16_t>(parameters, 76 + channel * 2) =
                field<std::uint16_t>(voice, 72 + channel * 2);
        }
        callback(220, mixer_set_rate);
        callback(232, mixer_set_pan);
        callback(236, mixer_seek);
        callback(240, mixer_set_loop);
        callback(224, mixer_set_voice_value);
        callback(252, mixer_position);
        field<Dword>(state, 244) = field<Dword>(state, 248) = 0;
        callback(216, mixer_stop);
        callback(228, mixer_set_effect);
        mixer_create(context, parameters);
    }
    backend_call<void>(state, 220, field<void *>(state, 184), field<std::uint16_t>(voice, 126));
    submit_volume(index);
    backend_call<void>(state, 232, field<void *>(state, 172), field<std::uint16_t>(voice, 24),
                       0);
    if (field<Dword>(state, 236))
        backend_call<void>(state, 236, field<void *>(state, 192), seek);
    if (field<Dword>(state, 240))
        backend_call<void>(state, 240, field<void *>(state, 192), flags);
    if (field<Dword>(state, 224))
        backend_call<void>(state, 224, field<void *>(state, 188),
                           field<std::uint16_t>(voice, 94));
    if (field<Dword>(state, 244))
        backend_call<void>(state, 244, field<void *>(state, 208), 100);
    const int result =
        (field<Byte>(voice, 32) & 16) ? device_start(context) : mixer_start(context);
    if (result < 0) {
        if (field<Byte>(voice, 32) & 2)
            clear_backend_chain(reinterpret_cast<Byte *>(state) + 100);
        return result;
    }
    backend_call<void>(state, 228, field<void *>(state, 180),
                       int(field<std::int16_t>(voice, 86)));
    if (field<Byte>(voice, 32) & 1)
        callback(256, device_stopped);
    return 0;
}
} // namespace nfs::sound::pending
