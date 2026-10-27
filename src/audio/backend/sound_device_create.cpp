#include "nfs/audio/backend/sound_backend.hpp"
#include <cstring>
namespace nfs::sound::backend {
extern const float device_rate_scale;
// Real DirectSound Create/Duplicate/QueryInterface and cleanup.
int device_create(void *context, const void *parameters) {
    field<volatile Dword>(context, 64) = 0;
    field<volatile Dword>(context, 68) = 0;
    field<volatile Dword>(context, 60) = 0;
    if (!caps_cached)
        query_device_limits(field<void *>(parameters, 0));
    void **buffer = &field<void *>(context, 60);
    if (!field<void *>(parameters, 4)) {
        alignas(4) Byte wave[18], description[36];
        for (unsigned word = 0; word < 9; ++word)
            field<volatile Dword>(description, word * 4) = 0;
        field<volatile std::uint16_t>(wave, 0) = 1;
        field<volatile std::uint16_t>(wave, 2) = 1;
        field<Dword>(wave, 4) = 22050;
        field<Dword>(wave, 8) = 44100;
        field<std::uint16_t>(wave, 12) = 2;
        field<std::uint16_t>(wave, 14) = 16;
        field<std::uint16_t>(wave, 16) = 0;
        field<Dword>(description, 0) = 36;
        field<Dword>(description, 4) = 32948;
        field<Dword>(description, 8) = 32768;
        field<void *>(description, 16) = wave;
        field<Dword>(context, 32) = 1;
        field<Dword>(context, 0) = reinterpret_cast<Dword>(device_refill);
        field<Dword>(context, 56) = Dword(-1);
        if (device_call<int>(field<void *>(parameters, 0), 12, description, buffer, 0)) {
            *buffer = nullptr;
            device_destroy(context);
            return 0;
        }
    } else {
        field<Dword>(context, 32) = 0;
        field<int>(context, 36) = field<std::int8_t>(parameters, 10);
        if (device_call<int>(field<void *>(parameters, 0), 20, field<void *>(parameters, 4),
                             buffer)) {
            *buffer = nullptr;
            device_destroy(context);
            return 0;
        }
    }
    void **queried = &field<void *>(context, 64);
    if (device_call<int>(*buffer, 0, buffer3d_iid, queried)) {
        *queried = nullptr;
        device_destroy(context);
        return 0;
    }
    if (field<Byte>(parameters, 11)) {
        queried = &field<void *>(context, 68);
        if (device_call<int>(*buffer, 0, effect_iid, queried)) {
            *queried = nullptr;
            device_destroy(context);
            return 0;
        }
    }
    field<float>(context, 28) =
        float(static_cast<long double>(field<std::uint16_t>(parameters, 8)) * device_rate_scale);
    return 1;
}
// Streaming and duplicated buffers retain their saved Play flags.
int device_start(void *context) {
    Dword flags = 2;
    if (field<Dword>(context, 32)) {
        void *memory;
        Dword length;
        device_call<int>(field<void *>(context, 60), 44, 0,
                         field<Dword>(context, 48) * 2, &memory, &length,
                         static_cast<void **>(nullptr), static_cast<Dword *>(nullptr), 0);
        field<Dword>(context, 52) = field<Dword>(context, 48);
        flags = 1;
        device_call<int>(field<void *>(context, 60), 76, memory, length,
                         static_cast<void *>(nullptr), Dword(0));
    }
    if (field<Dword>(context, 36))
        flags |= 1;
    if (device_call<int>(field<void *>(context, 60), 48, 0, 0, flags)) {
        device_destroy(context);
        return -1;
    }
    return 0;
}
} // namespace nfs::sound::backend
