#include "camera_controller_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include <cmath>
#include <initializer_list>
namespace nfs::game::world::camera {

void bind_follow_car(void *behavior, void *car) {
    at<void *>(behavior, 4) = car;
}

void bind_mode_three_body(void *behavior, void *body) {
    at<void *>(behavior, 4) = body;
}

void bind_mode_three_position(void *behavior, void *body) {
    at<void *>(behavior, 4) = body;
}

void set_mode_three_offset(void *behavior, Dword x, Dword y, float z) {
    at<Dword>(behavior, 8) = x;
    at<Dword>(behavior, 12) = y;
    at<float>(behavior, 16) = z;
}
namespace {
void bind_follow_mode(void *mode, void *car) {
    if (void *behavior = at<void *>(mode, 84))
        at<void *>(behavior, 4) = at<void *>(car, 1768);
    if (void *behavior = at<void *>(mode, 80))
        bind_follow_car(behavior, car);
    if (void *behavior = at<void *>(mode, 88))
        at<void *>(behavior, 36) = car;
    if (void *behavior = at<void *>(mode, 96))
        at<void *>(behavior, 4) = car;
}
} // namespace

void bind_mode_one(void *director, void *car) {
    bind_follow_mode(find_camera_mode(director, 1), car);
    camera_layout::refresh_mode_one(director);
}

void bind_mode_two(void *director, void *car) {
    bind_follow_mode(find_camera_mode(director, 2), car);
    camera_layout::refresh_mode_two(director);
}

void bind_mode_zero(void *director, void *car) {
    bind_follow_mode(find_camera_mode(director, 0), car);
    camera_layout::refresh_mode_zero(director);
}

void push_mode(void *director, Dword mode) {
    if (at<Dword>(director, 356))
        end_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    if (at<int>(director, 356) < 10)
        ++at<Dword>(director, 356);
    at<Dword>(director, 312 + at<Dword>(director, 356) * 4) = mode;
    begin_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    notify_camera_mode(director);
}
} // namespace nfs::game::world::camera
namespace nfs::game::world::pending {
// The saved argument is a car pointer represented by one stack word.
void set_camera_mode(void *director, int selected_car) {
    if (network::session_active() &&
        (at<std::int16_t>(subsystem(104), 10) < 4 || !at<Dword>(director, 300))) {
        camera::push_mode(director, 11);
        return;
    }
    void *car;
    if (!selected_car) {
        if (!at<Dword>(director, 300)) {
            car = runtime::physics::local_car(static_cast<Dword *>(subsystem(68)),
                                              at<int>(director, 304));
            at<void *>(director, 300) = car;
            if (!car) {
                const Dword count = at<Dword>(subsystem(72), 168);
                for (Dword index = 0; index < count; ++index)
                    if ((car = at<void *>(subsystem(68), 20 + index * 4)))
                        break;
                at<void *>(director, 300) = car;
            }
        }
        car = at<void *>(director, 300);
    } else {
        car = reinterpret_cast<void *>(selected_car);
        at<void *>(director, 300) = car;
    }
    if (!car)
        return;
    camera::bind_mode_one(director, car);
    camera::bind_mode_two(director, car);
    camera::bind_mode_zero(director, car);
    void *mode = find_camera_mode(director, 3), *body = at<void *>(car, 1768);
    if (void *behavior = at<void *>(mode, 84))
        camera::bind_mode_three_body(behavior, body);
    if (void *behavior = at<void *>(mode, 80)) {
        camera::bind_mode_three_position(behavior, body);
        using kernel::numeric::Extended;
        const auto format = kernel::numeric::current_format();
        const Extended size(at<float>(body, 128), format);
        auto distance = size - 1.f;
        const auto scaled = size * 0.85f;
        const float saved_scaled = float(scaled);
        if (scaled < distance)
            distance = Extended(saved_scaled, format);
        camera::set_mode_three_offset(behavior, 0, 1056964608, float(distance));
    }
    camera_layout::refresh_mode_three(director);
    for (Dword identifier : {8u, 9u, 4u}) {
        mode = find_camera_mode(director, identifier);
        if (void *behavior = at<void *>(mode, 84))
            at<void *>(behavior, 4) = offset(body, 680);
        if (identifier != 4)
            if (void *behavior = at<void *>(mode, 80))
                at<void *>(behavior, identifier == 8 ? 24 : 32) = body;
    }
    const unsigned player = at<Dword>(director, 304);
    if (!at<Dword>(subsystem(72), 140) && player < 2) {
        void *preferences = offset(subsystem(72), 2488 + player * 50088);
        const Dword preference = at<Dword>(preferences, 7548);
        camera::selected_modes[player] = preference >= 1 && preference <= 3 ? preference : 0;
    }
    camera::push_mode(director, camera::selected_modes[player]);
}
} // namespace nfs::game::world::pending
