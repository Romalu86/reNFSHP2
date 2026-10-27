#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend {
std::uint16_t caps_cached{};
std::int16_t minimum_frequency{};
int maximum_frequency{};
extern const float effect_scale, device_rate_scale;
extern const double position_scale;

void query_device_limits(void *device) {
    alignas(4) Byte caps[96];
    field<Dword>(caps, 0) = 96;
    device_call<int>(device, 16, caps);
    minimum_frequency = field<std::int16_t>(caps, 8);
    maximum_frequency = field<int>(caps, 12);
    caps_cached = 1;
}

void device_set_buffer_ms(void *context, int milliseconds) {
    field<int>(context, 44) = milliseconds;
    field<Dword>(context, 48) =
        Dword(int(Dword(field<int>(context, 40)) * Dword(milliseconds)) / 1000) & 16777200;
}

void device_set_pan(void *context, Dword horizontal, Dword vertical) {
    float p[3];
    voice_pan_vector(horizontal, vertical, p);
    for (float &v : p)
        v = float(static_cast<long double>(v) * position_scale);
    device_call<int>(field<void *>(context, 64), 76, p[0], p[1], p[2], 0);
}

void device_set_effect(void *context, int value) {
    if (void *effect = field<void *>(context, 68)) {
        float gain = float(static_cast<long double>(value) * effect_scale);
        device_call<int>(effect, 16, effect_guid, 1, 0, 0, &gain, 4);
    }
}
void device_set_rate(void *context, int value) {
    const long double scaled = static_cast<long double>(value) * field<float>(context, 28);
    int rate = int(Dword(game::runtime::hardware::truncate_integer_native80(&scaled)));
    if (rate < minimum_frequency)
        rate = minimum_frequency;
    else if (rate > maximum_frequency)
        rate = maximum_frequency;
    if (!device_call<int>(field<void *>(context, 60), 68, rate)) {
        field<int>(context, 40) = rate;
        field<int>(context, 48) = int(Dword(field<int>(context, 44)) * Dword(rate)) / 1000;
    }
}

Dword device_position(void *context) {
    Dword position = reinterpret_cast<Dword>(context);
    device_call<int>(field<void *>(context, 60), 16, &position, static_cast<Dword *>(nullptr));
    return position >> 1;
}

Dword device_stopped(void *context) {
    Dword status = reinterpret_cast<Dword>(context);
    device_call<int>(field<void *>(context, 60), 36, &status);
    return ~status & 1;
}

int device_remaining(void *context) {
    Dword position = reinterpret_cast<Dword>(context);
    device_call<int>(field<void *>(context, 60), 16, static_cast<Dword *>(nullptr), &position);
    int count = int(Dword(field<int>(context, 48)) - Dword(field<int>(context, 52)) +
                    ((position >> 1) & 268435440));
    if (count >= 16384)
        count -= 16384;
    if (count < 0)
        count = 0;
    return count;
}
// Callbacks release each nonnull interface in this order.
void device_destroy(void *context) {
    void *buffer = field<void *>(context, 60);
    if (buffer)
        device_call<int>(buffer, 72);
    for (unsigned offset : {68u, 64u, 60u})
        if (void *object = field<void *>(context, offset))
            device_call<Dword>(object, 8);
}
} // namespace nfs::sound::backend
