#include "../events/game_input_event_runtime.hpp"
#include "../../ai/population/ai_population_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::services::input_events {
using game::runtime::at;
using game::runtime::bytes;

void store_steering_input(float *destination, int value) {
    *destination = static_cast<float>(static_cast<long double>(value) * 0.0078740157186985015869140625f);
}

void store_reset_speed(float *destination, float speed) {
    *destination = static_cast<float>(static_cast<long double>(speed) * 0.44703f);
}

Dword request_previous_gear(void *car) {
    Dword gear = at<Dword>(car, 96);
    if (at<Byte>(car, 776))
        return gear;
    if (int(gear) > 1) {
        at<Byte>(car, 1586) = Byte(gear - 1);
        return gear - 1;
    }
    if (gear == 1) {
        float speed = at<float>(car, 92);
        if (speed < 0)
            speed = -speed;
        if (speed <= 50)
            gear = 0;
    }
    at<Byte>(car, 1586) = Byte(gear);
    return gear;
}

Dword request_gear(void *car, int requested) {
    if (at<Byte>(car, 776))
        return at<Dword>(car, 96);
    const int last = int(at<Dword>(car, 780) - 1);
    if (requested < 0)
        requested = 0;
    else if (last < requested)
        requested = last;
    auto absolute_speed = [&] {
        float value = at<float>(car, 92);
        return value < 0 ? -value : value;
    };
    if (!requested && at<Dword>(car, 96) && absolute_speed() > 50)
        requested = at<int>(car, 96);
    const long double rpm =
        static_cast<long double>(at<float>(car, 788 + Dword(requested) * 4)) *
        absolute_speed() * 9.54929637908935546875f;
    const float saved_rpm = static_cast<float>(rpm);
    const int integer_rpm = game::runtime::hardware::current_integer_f32(saved_rpm);
    return integer_rpm <= at<int>(car, 28) ? Dword(requested) : at<Dword>(car, 96);
}
// Callers use only the low byte of the return value.
Byte reset_allowed(void *entity) {
    auto chase_of = [&]() -> void * {
        return at<void *>(entity, reinterpret_cast<const int *>(at<Dword>(entity, 4))[4] + 12);
    };
    void *chase = chase_of();
    if (!chase)
        return 1;
    auto too_close = [&](void *other, float limit) {
        const long double distance = game::runtime::ai::pair_distance(entity, other);
        return distance < limit;
    };
    for (Dword index = 0; index < at<Dword>(chase, 40); ++index) {
        if (too_close(at<void *>(chase, 28 + index * 4), 10))
            return 0;
        chase = chase_of();
    }
    if (at<int>(chase, 96) >= 0) {
        void *location = bytes(game::runtime::ai::lifecycle::state<void *>(7353292)) +
                         Dword(at<int>(chase, 96)) * 64;
        for (Dword index = 0; index < at<Dword>(location, 32); ++index)
            if (too_close(at<void *>(location, 36 + index * 4), 10))
                return 0;
        if (void *roadblock = at<void *>(location, 44))
            if (too_close(roadblock, 30))
                return 0;
    }
    return 1;
}

int cycle_camera(void *director) {
    using namespace game::world;
    if (at<Byte>(subsystem(48), 104) || at<Byte>(director, 172))
        return -1;
    auto active_mode = [&]() -> Dword & {
        return at<Dword>(director, 312 + at<Dword>(director, 356) * 4);
    };
    const Dword previous = active_mode();
    if (previous > 3)
        return -1;
    static const Dword following[4] = {3, 0, 1, 2};
    const Dword selected = following[previous];
    end_camera_mode(director, active_mode());
    --at<Dword>(director, 356);
    if (at<Dword>(director, 356))
        begin_camera_mode(director, active_mode());
    notify_camera_mode(director);
    if (at<Dword>(director, 356))
        end_camera_mode(director, active_mode());
    if (at<int>(director, 356) < 10)
        ++at<Dword>(director, 356);
    active_mode() = selected;
    begin_camera_mode(director, active_mode());
    notify_camera_mode(director);
    const Dword player = at<Dword>(director, 304);
    if (player < 2) {
        void *preferences = offset(subsystem(72), 2488 + player * 50088);
        if (preferences)
            at<Dword>(preferences, 7548) = selected;
    }
    return int(selected);
}
} // namespace nfs::services::input_events
