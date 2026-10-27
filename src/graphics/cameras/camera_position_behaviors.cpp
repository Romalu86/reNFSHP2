#include "camera_director_runtime.hpp"
namespace nfs::game::world::camera {
using kernel::numeric::Extended;

void __fastcall advance_mount_position(void *behavior, void *, void *state) {
    void *director = at<void *>(state, 0), *body = at<void *>(behavior, 4);
    float local[3];
    std::memcpy(local, offset(behavior, 8), 12);
    at<Dword>(behavior, 12) = at<Dword>(behavior, 20);
    if (director) {
        if (at<Byte>(director, 164))
            local[2] = -at<float>(body, 128);
        else if (at<Byte>(director, 165)) {
            local[0] = -at<float>(body, 120);
            local[2] = 0;
        } else if (at<Byte>(director, 166)) {
            local[0] = at<float>(body, 120);
            local[2] = 0;
        }
    }
    float transformed[3], position[3];
    runtime::matrices::transform_vectors(1, local, reinterpret_cast<float *>(offset(body, 692)),
                                         transformed);
    const auto format = kernel::numeric::current_format();
    for (unsigned axis = 0; axis < 3; ++axis)
        position[axis] =
            float(Extended(transformed[axis], format) + at<float>(body, 680 + axis * 4));
    void *ground = offset(behavior, 36);
    clip_camera_position(ground, position, body, 1.5f, 0);
    raise_camera_above_ground(ground, position, 1.f);
    refresh_camera_ground(position, at<void *>(behavior, 4), ground);
    std::memcpy(offset(state, 4), position, 12);
}

void __fastcall advance_matrix_position(void *behavior, void *, void *state) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    const auto next_phase = X(at<float>(behavior, 12)) + at<float>(behavior, 8);
    at<float>(behavior, 8) = float(next_phase);
    if (!(next_phase < X(0.75f)) && !std::isnan(double(next_phase))) {
        at<float>(behavior, 8) = -0.25f;
        pop_matrix_mode(at<void *>(state, 0));
    }
    float phase = at<float>(behavior, 8), sine, cosine;
    runtime::turn_sine_cosine(&phase, &sine, &cosine);
    float local[3];
    local[0] = float((X(at<float>(behavior, 20)) + at<float>(behavior, 16)) * cosine);
    local[2] = float((X(at<float>(behavior, 28)) + at<float>(behavior, 16)) * sine);
    phase = float(X(at<float>(behavior, 8)) * 0.5f);
    runtime::turn_sine_cosine(&phase, &sine, &cosine);
    local[1] = float(X(sine) + at<float>(behavior, 24));
    void *body = at<void *>(behavior, 32);
    float transformed[3], position[3];
    runtime::matrices::transform_vectors(1, local, reinterpret_cast<float *>(offset(body, 692)),
                                         transformed);
    for (unsigned axis = 0; axis < 3; ++axis)
        position[axis] = float(X(transformed[axis]) + at<float>(body, 680 + axis * 4));
    void *ground = offset(behavior, 36);
    clip_camera_position(ground, position, body, 1.5f, 0);
    raise_camera_above_ground(ground, position, 1.f);
    void *retained_body = at<void *>(behavior, 32);
    avoid_camera_vehicles(position, runtime::physics::vehicle_car(retained_body), 1.5f);
    refresh_camera_ground(position, retained_body, ground);
    std::memcpy(offset(state, 4), position, 12);
}

void __fastcall advance_pause_position(void *behavior, void *, void *state) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    const auto next_phase = X(at<float>(behavior, 8)) + at<float>(behavior, 4);
    at<float>(behavior, 4) = float(next_phase);
    if (!std::isnan(double(next_phase)) &&
        (!(next_phase < X(1.f)) || !(X(-1.f) < next_phase)))
        at<Dword>(behavior, 4) = 0;
    float phase = at<float>(behavior, 4), sine, cosine;
    runtime::turn_sine_cosine(&phase, &sine, &cosine);
    const float x = float(X(cosine) * at<float>(behavior, 12));
    const float z = float(X(sine) * at<float>(behavior, 20));
    phase = float(X(at<float>(behavior, 4)) * 0.5f);
    runtime::turn_sine_cosine(&phase, &sine, &cosine);
    void *body = at<void *>(behavior, 24);
    float position[] = {float(X(x) + at<float>(body, 680)),
                        float(X(sine) + at<float>(behavior, 16) + at<float>(body, 684)),
                        float(X(z) + at<float>(body, 688))};
    void *ground = offset(behavior, 28);
    clip_camera_position(ground, position, body, 2.f, 0);
    raise_camera_above_ground(ground, position, 1.f);
    void *retained_body = at<void *>(behavior, 24);
    avoid_camera_vehicles(position, runtime::physics::vehicle_car(retained_body), 2.f);
    refresh_camera_ground(position, retained_body, ground);
    std::memcpy(offset(state, 4), position, 12);
}
} // namespace nfs::game::world::camera
