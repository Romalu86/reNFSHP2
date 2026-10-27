#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::race_audio {
namespace pending {

void reset_listener(Dword listener) {
    if (Dword *object = listener_objects[listener])
        object[1] = 0;
}

void update_engines() {
    Byte *state = listener_state + listener_index * 2760;
    at<volatile Dword>(state, 384) = 0;
    at<volatile Dword>(state, 580) = 0;
    for (unsigned i = 0; i < 12; ++i)
        at<Dword>(state, 2120 + i * 56) = 10000;
}

void update_impacts() {
    Byte *state = listener_state + listener_index * 2760;
    at<Dword>(state, 220) = 0;
    for (unsigned i = 0; i < 10; ++i) {
        at<volatile Dword>(state, 232 + i * 16) = 10000;
        at<volatile Dword>(state, 224 + i * 16) = 0;
    }
}

void set_listener(void *position, void *output) {
    if (at<Dword>(system(72), 140)) {
        float *velocity = race_audio_listener_velocity + listener_index * 3;
        for (unsigned i = 0; i < 3; ++i)
            at<volatile float>(velocity, i * 4) =
                float((static_cast<long double>(at<float>(position, i * 4)) -
                       previous_listener[i]) * 32.f);
        at<volatile Dword>(previous_listener, 0) = at<volatile Dword>(position, 0);
        const Dword previous_y = at<volatile Dword>(position, 4);
        const Dword previous_z = at<volatile Dword>(position, 8);
        at<volatile Dword>(previous_listener, 4) = previous_y;
        at<volatile Dword>(previous_listener, 8) = previous_z;
        *static_cast<float **>(output) = velocity;
    } else {
        void *car = at<void *>(listener_state, 2792 + listener_index * 2760);
        *static_cast<float **>(output) =
            reinterpret_cast<float *>(bytes(at<void *>(car, 1768)) + 8);
    }
}
} // namespace pending Source/listener dot products are rounded at stores.
 
int doppler(const float *position, const float *source_velocity, const float *listener_velocity) {
    using namespace pending;
    float direction[3];
    for (unsigned i = 0; i < 3; ++i)
        at<volatile float>(direction, i * 4) =
            float(static_cast<long double>(position[i]) - at<float>(listener_state, i * 4));
    hardware::normalize_collision(direction);
    volatile float shift = 0;
    if (source_velocity)
        shift = float(hardware::dot_product(direction, source_velocity));
    if (listener_velocity) {
        long double projected = hardware::dot_product(direction, listener_velocity);
        if (projected < -30.f)
            projected = -30.f;
        else if (30.f < projected)
            projected = 30.f;
        shift = float(static_cast<long double>(shift) - projected);
    }
    long double ratio = 220.0L / (static_cast<long double>(shift) + 220.f);
    if (ratio < 0.f)
        ratio = 0.f;
    else if (2.f < ratio)
        ratio = 2.f;
    const volatile float fixed_ratio = float(ratio * 65536.f);
    return hardware::current_integer_f32(fixed_ratio);
}

void pending::update_emitter(void *emitter) {
    float separation[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        at<volatile float>(separation, axis * 4) =
            float(static_cast<long double>(at<float>(emitter, axis * 4)) -
                  at<float>(listener_state, axis * 4));
    const volatile float measured_distance = float(hardware::vector_length(separation));
    int distance = hardware::current_integer_f32(measured_distance);
    const int listener = listener_index;
    if (distance < 0 || distance > 10000)
        distance = 10000;
    Dword squared = Dword(distance) * Dword(distance), radius = at<Dword>(emitter, 36);
    if (static_cast<int>(radius * radius) <= static_cast<int>(squared))
        return;
    Byte *state = listener_state + listener_index * 2760;
    int slot = 0;
    while (at<int>(state, 2120 + slot * 56) <= distance) {
        if (++slot > 11)
            break;
    }
    if (slot < 12) {
        for (int i = 11; i > slot; --i)
            for (unsigned word = 0; word < 14; ++word)
                at<Dword>(state, 2120 + i * 56 + word * 4) =
                    at<Dword>(state, 2120 + (i - 1) * 56 + word * 4);
        Byte *record = state + 2120 + slot * 56;
        at<int>(record, 0) = distance;
        at<Dword>(record, 4) = squared;
        // First XYZ, serial, then velocity XYZ and the five remaining fields.
        for (unsigned i = 0; i < 3; ++i)
            at<Dword>(record, 8 + i * 4) = at<Dword>(emitter, i * 4);
        at<Dword>(record, 32) = at<Dword>(emitter, 24);
        for (unsigned i = 3; i < 6; ++i)
            at<Dword>(record, 8 + i * 4) = at<Dword>(emitter, i * 4);
        for (unsigned i = 7; i < 12; ++i)
            at<Dword>(record, 8 + i * 4) = at<Dword>(emitter, i * 4);
    }
    int &count = at<int>(listener_state, 384 + listener * 2760);
    count = static_cast<int>(Dword(count) + 1);
    if (count > 12)
        count = 12;
}

void pending::update_nearest() {
    Byte *state = listener_state + listener_index * 2760;
    const int distance = at<int>(state, 2832);
    if (distance > 150)
        return;
    float position[3], velocity[3];
    at<volatile Dword>(position, 0) = at<volatile Dword>(state, 2808);
    const Dword position_y = at<volatile Dword>(state, 2812);
    const Dword position_z = at<volatile Dword>(state, 2816);
    at<volatile Dword>(position, 4) = position_y;
    const Dword velocity_x = at<volatile Dword>(state, 2820);
    at<volatile Dword>(position, 8) = position_z;
    const Dword velocity_y = at<volatile Dword>(state, 2824);
    const Dword velocity_z = at<volatile Dword>(state, 2828);
    at<volatile Dword>(velocity, 0) = velocity_x;
    at<volatile Dword>(velocity, 4) = velocity_y;
    at<volatile Dword>(velocity, 8) = velocity_z;
    Dword pitch = Dword(
        doppler(position, velocity, reinterpret_cast<float *>(listener_configuration)));
    Dword pan = spatial_pan(listener_state, position),
         channel = Dword(listener_index);
    if (race_audio_listener_count == 2)
        pan = listener_index ? 16383 : 49152;
    // Split-screen listener zero takes the saved early branch before normalizing the channel.
    if (!(race_audio_listener_count == 2 && listener_index == 0) && listener_index)
        channel = 256;
    submit_nearest(channel | 10, 5, 64, pitch, Dword(distance) * Dword(distance), pan);
    event_flags[10 + listener_index * 124] = 1;
}
} // namespace nfs::game::runtime::race_audio
