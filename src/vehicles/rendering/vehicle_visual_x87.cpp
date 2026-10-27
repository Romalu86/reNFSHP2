#include "vehicle_renderable_frame.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cstring>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Vehicle visual arithmetic requires native x87 intermediates");
namespace nfs::game::runtime::ai::cars { Dword car_is_traffic(void *); }
namespace nfs::game::world::vehicle_render {
namespace {
// This scalar return keeps the saved floating load distinct from a raw word copy.
__declspec(noinline) float load_damage_limit(const volatile float *source) {
    return *source;
}
}

void reset_damage(void *owner) {
    if (!at<Byte>(owner, 20))
        return;
    void *zones = at<void *>(owner, 76);
    const Dword maximum = launch::field<Dword>(8627920);
    for (unsigned index = 0; index < 10; ++index) {
        void *zone = offset(zones, index * 72);
        at<volatile Byte>(zone, 53) = 0;
        at<volatile Dword>(zone, 60) = 0;
        if (maximum)
            at<volatile float>(zone, 60) = load_damage_limit(&at<float>(zone, 64));
    }
}

void set_light_channel(void *owner, Dword channel, Dword state) {
    void *selected = offset(owner, 72 + channel * 168);
    if (at<Dword>(selected, 16) == state)
        return;
    at<Dword>(selected, 16) = state;
    const int count = at<int>(selected, 20);
    if (count > 0) {
        const auto ticks = static_cast<long double>(std::int32_t(graphics::unresolved::camera_state));
        void *glows = at<void *>(owner, 744);
        const auto elapsed = ticks * 0.01666666753590106964111328125f;
        for (int index = 0; index < count; ++index) {
            const Dword glow = at<Dword>(selected, 52 + Dword(index) * 4);
            at<volatile float>(glows, glow * 248 + 216) = float(elapsed);
        }
    }
}


static long double body_surface_brightness_retained(void *body) {
    void *surface = at<void *>(body, 532);
    if (!surface)
        return 128.0L;
    const int first = at<Byte>(surface, 51);
    const int second = at<Byte>(surface, 50);
    const int third = at<Byte>(surface, 49);
    const int fourth = at<Byte>(surface, 48);
    return ((static_cast<long double>(first) + second) + third + fourth) * .25f;
}
namespace visual_x87 {
void body_surface_brightness_native80(void *body, void *output) {
    const long double value = body_surface_brightness_retained(body);
    std::memcpy(output, &value, 10);
}
}

void smooth_renderable_damage(void *model, Byte smooth) {
    if (at<Byte>(subsystem(52), 81296))
        return;
    const float target = float(
        body_surface_brightness_retained(at<void *>(at<void *>(model, 2124), 1768)) * 0.0039215688593685626983642578125f);
    const Byte initialized = at<Byte>(model, 2072);
    if (!initialized || !smooth) {
        Dword target_bits;
        std::memcpy(&target_bits, &target, 4);
        at<volatile Dword>(model, 2076) = target_bits;
        at<volatile Dword>(model, 2080) = target_bits;
        at<volatile Dword>(model, 2084) = target_bits;
        if (!initialized)
            at<Byte>(model, 2072) = 1;
        at<volatile Dword>(model, 2088) = 0;
        return;
    }
    const float current = at<float>(model, 2076);
    if (current < target) {
        const auto next = static_cast<long double>(current) + 0.0392156876623630523681640625f;
        at<float>(model, 2076) = float(next);
        if (static_cast<long double>(target) < next)
            at<float>(model, 2076) = target;
    } else if (target < current) {
        const auto next = static_cast<long double>(current) - 0.0392156876623630523681640625f;
        at<float>(model, 2076) = float(next);
        if (next < static_cast<long double>(target))
            at<float>(model, 2076) = target;
    }
}

void update_renderable_lod(void *model, void *director, const float *position) {
    void *settings = subsystem(52);
    if (at<Byte>(settings, 81296)) {
        at<volatile float>(model, 2120) =
            float(static_cast<long double>(at<float>(settings, 81312)));
        return;
    }
    float eye[3];
    camera::active_camera_position(director, eye);
    eye[2] = -eye[2];
    auto X = [](float value) { return static_cast<long double>(value); };
    const auto x = X(eye[0]) - position[0], y = X(eye[1]) - position[1],
               z = X(eye[2]) - position[2];
    const float square = float(z * z + y * y + x * x);
    const float distance = float(__builtin_sqrtl(X(square)));
    const float range = at<int>(subsystem(72), 2384) > 0 ? 100.f : 50.f;
    auto value = X(100) - (X(distance) * 100.f) / X(range);
    void *owner = subsystem(52);
    const float limit = at<volatile float>(owner, 81308);
    if (limit < 100.f) {
        const float selected_limit = at<volatile float>(owner, 81308);
        if (X(selected_limit) < value)
            value = X(selected_limit);
    } else if (X(0) < value)
        value = value - (X(100) - at<float>(owner, 81312));
    if (value < X(1))
        value = X(1);
    const float stored = float(value);
    at<volatile float>(model, 2120) = stored;
    if (runtime::ai::cars::car_is_traffic(at<void *>(model, 2124))) {
        value = X(stored) + 10.f;
        at<volatile float>(model, 2120) = float(value);
        if (X(100) < value)
            value = X(100);
        at<volatile float>(model, 2120) = float(value);
    }
}
} // namespace nfs::game::world::vehicle_render
