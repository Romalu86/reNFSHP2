#include "vehicle_feedback_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {
void emit_sparks(void *, const float *, float, const float *);
namespace {
int scaled_feedback(int amount, Dword rule) {
    return int(Dword(amount) * network::packets::rule_value(system(72), int(rule))) / 100;
}
void retain_feedback(void *car, int first, int second) {
    const int previous_first = at<volatile int>(car, 7996);
    at<volatile int>(car, 7996) = previous_first > first ? previous_first : first;
    const int previous_second = at<volatile int>(car, 8000);
    at<volatile int>(car, 8000) = previous_second > second ? previous_second : second;
}
// Keep the four rule queries in order.
void accumulate_collision_feedback(void *car, int amount) {
    const int first = scaled_feedback(amount, 53);
    const int second = scaled_feedback(amount, 53);
    const int first_scaled = scaled_feedback(first, 55);
    const int second_scaled = scaled_feedback(second, 55);
    retain_feedback(car, first_scaled, second_scaled);
}
int feedback_setting(unsigned address) {
    return at<int>(services::pending::rumble_values, address - 6899704);
}
bool sparks_on_surface(Dword surface) {
    switch (surface) {
    case 0:
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
    case 19:
    case 20:
    case 21:
    case 22:
    case 29:
    case 30:
        return true;
    default:
        return false;
    }
}
} // namespace

void accumulate_rumble(void *car, int first, int second, int rule) {
    first = scaled_feedback(first, 53);
    second = scaled_feedback(second, 53);
    if (rule >= 54 && rule <= 57) {
        first = scaled_feedback(first, Dword(rule));
        second = scaled_feedback(second, Dword(rule));
        retain_feedback(car, first, second);
    }
}
// The unordered input passes through unchanged.
long double limit_impact_shake(float value, float minimum, float maximum) {
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}
} // namespace nfs::game::runtime::physics::contacts_pending
namespace nfs::game::world::camera {

void begin_impact_shake(void *behavior, const float *direction, float strength) {
    if (at<Dword>(behavior, 472))
        return;
    const long double amplitude =
        static_cast<long double>(strength) * at<volatile float>(behavior, 264);
    const auto *direction_bits = reinterpret_cast<const volatile Dword *>(direction);
    at<volatile Dword>(behavior, 480) = direction_bits[0];
    const Dword second = direction_bits[1];
    at<volatile float>(behavior, 476) = static_cast<float>(amplitude);
    const Dword third = direction_bits[2];
    const long double duration =
        static_cast<long double>(strength) * at<volatile float>(behavior, 268);
    at<volatile Dword>(behavior, 484) = second;
    at<volatile Dword>(behavior, 488) = third;
    at<Dword>(behavior, 472) =
        Dword(game::runtime::hardware::truncate_integer_native80(&duration));
}
// A null active controller retains the saved invalid dereference.
void notify_impact_shake(void *director, const float *direction, float strength) {
    void *controller = invoke<void *>(director, 2);
    const Dword mode = at<Dword>(controller, 72);
    if (mode == 0 || mode == 1) {
        void *entry = find_camera_mode(director, mode);
        begin_impact_shake(at<void *>(entry, 80), direction, strength);
    }
}
} // namespace nfs::game::world::camera
namespace nfs::game::runtime::physics::contacts_pending {
// The scalar return preserves the original floating load before FCHS/FST32.
__declspec(noinline) float load_impact_coordinate(const volatile float *value) {
    return *value;
}

// Body collision sparks, rumble, audio event and camera shake.
void update_collision_feedback(void *car) {
    auto retained = [](float value) { return static_cast<long double>(value); };
    void *body = at<void *>(car, 1768);
    if (at<float>(body, 608) == 0 || at<float>(body, 660) == 0)
        return;
    const Dword kind = at<Dword>(body, 652);
    float point[3];
    std::memcpy(point, bytes(body) + 640, sizeof(point));
    static_cast<volatile float &>(point[2]) = -load_impact_coordinate(point + 2);
    const volatile float ratio =
        static_cast<float>(retained(at<float>(body, 608)) / at<float>(body, 660));
    void *other =
        is_local_vehicle(at<void *>(body, 604)) ? vehicle_car(at<void *>(body, 604)) : nullptr;
    if (kind == 1) {
        if (sparks_on_surface(at<Dword>(body, 656)) && at<Dword>(body, 636) == 2 &&
            at<float>(body, 592) > 1) {
            const long double speed = hardware::vector_length(&at<float>(body, 624));
            const float strength = float((retained(5) / retained(at<float>(body, 660))) * speed);
            emit_sparks(car, point, strength, nullptr);
            auto sound = retained(at<float>(car, 3168)) * 2.2369899749755859375f * 0.008333333767950534820556640625f;
            if (sound < retained(0))
                sound = sound * -1.f;
            const int event_index = at<volatile int>(car, 3208);
            const Dword surface = at<volatile Dword>(body, 656);
            if (event_index < 8) {
                if (sound < retained(0))
                    sound = retained(0);
                else if (retained(10) < sound)
                    sound = retained(10);
                sound = sound * 10.f;
                at<volatile Dword>(car, 3212 + Dword(event_index) * 20u) = 7;
                at<volatile Dword>(car, 3216 + at<volatile Dword>(car, 3208) * 20u) = 15;
                const Dword surface_offset = (at<volatile Dword>(car, 3208) + 161u) * 20u;
                const volatile float sample = static_cast<float>(sound * 65536.f);
                at<volatile Dword>(car, surface_offset) = surface;
                const Dword gain = Dword(hardware::current_integer_f32(sample));
                at<volatile Dword>(car, 3224 + at<volatile Dword>(car, 3208) * 20u) = gain;
                at<volatile Dword>(car, 3228 + at<volatile Dword>(car, 3208) * 20u) = 0;
                ++at<volatile Dword>(car, 3208);
            }
            if (other)
                accumulate_collision_feedback(other, feedback_setting(6899732));
            accumulate_collision_feedback(car, feedback_setting(6899732));
        }
        if (ratio > 10)
            accumulate_collision_feedback(car, feedback_setting(6899728));
        else if (ratio > 20)
            accumulate_rumble(car, feedback_setting(6899724), feedback_setting(6899724), 55);
    } else if (kind == 2) {
        void *other_body = at<void *>(body, 604);
        if (other_body && vehicle_car(other_body)) {
            const unsigned setting = ratio > 20 ? 6899724 : 6899728;
            if (ratio > 20)
                emit_sparks(car, point, float(retained(ratio) * .025f), nullptr);
            if (other)
                accumulate_collision_feedback(other, feedback_setting(setting));
            accumulate_collision_feedback(car, feedback_setting(setting));
        } else if (ratio > 20) {
            // The fallback reads the contact's other body again after the lookup.
            other_body = at<void *>(body, 604);
            if (other_body && sparks_on_surface(at<Dword>(other_body, 40)))
                emit_sparks(car, point, float(retained(ratio) * .025f), nullptr);
        }
    } else if (ratio > 3) {
        if (other)
            accumulate_rumble(other, feedback_setting(6899736), feedback_setting(6899736), 55);
        accumulate_rumble(car, feedback_setting(6899736), feedback_setting(6899736), 55);
    }
    if (ratio > 10 && (!at<void *>(body, 604) || vehicle_car(at<void *>(body, 604))) &&
        audio::vehicle::car_audio_suppressed(car)) {
        volatile float strength = float(retained(ratio) * 0.0333333350718021392822265625f);
        if (at<void *>(body, 604))
            strength = float(retained(strength) + strength);
        if (tick_services[at<Dword>(car, 1776)]) {
            strength = float(limit_impact_shake(strength, 0, 1));
            world::camera::notify_impact_shake(tick_services[at<Dword>(car, 1776)],
                                               &at<float>(body, 664), strength);
        }
    }
}
} // namespace nfs::game::runtime::physics::contacts_pending
