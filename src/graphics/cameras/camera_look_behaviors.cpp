#include "camera_director_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::entities::pending {
kernel::numeric::Extended direction_turns(float, float);
}
namespace nfs::game::world::camera {
using kernel::numeric::Extended;
namespace {
Extended magnitude(Extended value) {
    return value < Extended(0.f, kernel::numeric::current_format()) ? -value : value;
}
void build_look_basis(const float *up, const float *direction, float pitch, float *basis) {
    runtime::cross_vectors_entry(up, direction, basis);
    runtime::normalize_vector(basis, basis);
    runtime::cross_vectors_entry(direction, basis, basis + 3);
    std::memcpy(basis + 6, direction, 12);
    pitch_basis(basis, pitch, basis);
}
} // namespace

void __fastcall advance_follow_look(void *behavior, void *, void *state) {
    void *body = at<void *>(behavior, 4);
    at<Dword>(behavior, 124) = Dword(-2);
    if (!runtime::physics::vehicle_car(body))
        return;
    float up[3], camera_position[3], target[3];
    std::memcpy(up, offset(state, 60), 12);
    std::memcpy(camera_position, offset(state, 4), 12);
    std::memcpy(target, offset(body, 680), 12);
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    if (at<Dword>(state, 72) != 2) {
        float delta[3], local[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            delta[axis] = float(X(camera_position[axis]) - target[axis]);
        runtime::matrices::transform_vectors(1, delta,
                                             reinterpret_cast<float *>(offset(body, 72)), local);
        const auto body_up = magnitude(X(at<float>(body, 112)));
        auto direction =
            magnitude(runtime::ai::entities::pending::direction_turns(local[0], -local[2]));
        direction = (X(1.f) - body_up * 0.5f) * direction;
        direction = magnitude(direction + direction);
        const float weight = float(direction);
        float bounded = weight;
        if (weight < 0.f)
            bounded = 0.f;
        else if (1.f < weight)
            bounded = 1.f;
        at<float>(behavior, 132) = float(
            (X(bounded) * at<float>(body, 128) - at<float>(behavior, 132)) * 0.0333333350718021392822265625f +
            at<float>(behavior, 132));
        const float offset_vector[3] = {0, 0, at<float>(behavior, 132)};
        runtime::matrices::transform_vectors(1, offset_vector,
                                             reinterpret_cast<float *>(offset(body, 692)), local);
        for (unsigned axis = 0; axis < 3; ++axis)
            target[axis] = float(X(local[axis]) + target[axis]);
    }
    float direction[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        direction[axis] = float(X(target[axis]) - camera_position[axis]);
    const auto inverse = X(1.f) / runtime::retained_vector_length(direction);
    for (unsigned axis = 0; axis < 3; ++axis)
        direction[axis] = float(X(direction[axis]) * inverse);
    float alignment = float(runtime::retained_dot_product(up, direction));
    if (alignment < 0)
        alignment = -alignment;
    if (0.99f < alignment) {
        const float forward[3] = {0, 0, 10};
        float transformed[3];
        runtime::matrices::transform_vectors(
            1, forward, reinterpret_cast<float *>(offset(body, 692)), transformed);
        for (unsigned axis = 0; axis < 3; ++axis) {
            transformed[axis] = float(X(transformed[axis]) + at<float>(body, 680 + axis * 4));
            direction[axis] = float(X(at<float>(body, 680 + axis * 4)) - camera_position[axis]);
        }
        runtime::physics::pending::normalize_collision_entry(direction);
    }
    float basis[9];
    build_look_basis(up, direction, at<float>(behavior, 8), basis);
    if (at<Dword>(state, 0) && at<Byte>(at<void *>(state, 0), 164))
        at<Dword>(behavior, 124) = Dword(-1);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(state, 16 + axis * 4) = float(X(basis[6 + axis]) + camera_position[axis]);
}

void set_mount_look_vector(void *behavior, Dword x, Dword y, Dword z) {
    at<Dword>(behavior, 8) = x;
    at<Dword>(behavior, 12) = y;
    at<Dword>(behavior, 16) = z;
}

void __fastcall advance_mount_look(void *behavior, void *, void *state) {
    float position[3], local[3];
    std::memcpy(position, offset(state, 4), 12);
    std::memcpy(local, offset(behavior, 8), 12);
    void *director = at<void *>(state, 0);
    void *body = at<void *>(behavior, 4);
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    if (director) {
        if (at<Byte>(director, 164) == 1) {
            local[0] = local[1] = 0;
            local[2] = float(X(at<float>(body, 128)) * -2.f);
        } else if (at<Byte>(director, 165)) {
            local[0] = float(X(at<float>(body, 120)) * -2.f);
            local[1] = local[2] = 0;
        } else if (at<Byte>(director, 166)) {
            local[0] = float(X(at<float>(body, 120)) * 2.f);
            local[1] = local[2] = 0;
        }
    }
    if (at<Dword>(director, 312 + at<Dword>(director, 356) * 4) == 3) {
        float basis[9], direction[3], body_rotation[9];
        std::memcpy(body_rotation, offset(body, 692), 36);
        std::memcpy(offset(state, 60), body_rotation + 3, 12);
        for (unsigned axis = 0; axis < 3; ++axis)
            direction[axis] = float(X(at<float>(body, 680 + axis * 4)) - position[axis]);
        const auto inverse = X(1.f) / runtime::retained_vector_length(direction);
        for (float &value : direction)
            value = float(X(value) * inverse);
        float alignment = float(runtime::retained_dot_product(body_rotation + 3, direction));
        if (alignment < 0)
            alignment = -alignment;
        if (0.99f < alignment) {
            const float forward[3] = {0, 0, 10};
            float unused[3];
            runtime::matrices::transform_vectors(1, forward, body_rotation, unused);
            for (unsigned axis = 0; axis < 3; ++axis) {
                unused[axis] = float(X(unused[axis]) + at<float>(body, 680 + axis * 4));
                direction[axis] = float(X(at<float>(body, 680 + axis * 4)) - position[axis]);
            }
            runtime::physics::pending::normalize_collision_entry(direction);
        }
        build_look_basis(body_rotation + 3, direction, at<float>(behavior, 20), basis);
    } else {
        at<Dword>(state, 60) = at<Dword>(state, 68) = 0;
        at<float>(state, 64) = 1;
    }
    float transformed[3];
    runtime::matrices::transform_vectors(1, local, reinterpret_cast<float *>(offset(body, 692)),
                                         transformed);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(state, 16 + axis * 4) = float(X(transformed[axis]) + position[axis]);
}

void __fastcall advance_follow_up(void *behavior, void *, void *state) {
    void *car = at<void *>(behavior, 4);
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto roll = -(X(at<float>(car, 3200)) * at<float>(car, 3168) * 0.f) +
                at<float>(behavior, 20);
    roll = roll - roll * at<float>(behavior, 24);
    at<float>(behavior, 20) = float(roll);

    float angle = float(roll * roll * roll * 0.001f * 10.f);
    if (roll < X(0.f))
        angle = -angle;
    const void *body = at<void *>(car, 1768);
    float direction[] = {float(X(at<float>(state, 4)) - at<float>(body, 680)), 0,
                         float(X(at<float>(state, 12)) - at<float>(body, 688))};
    runtime::physics::pending::normalize_collision_entry(direction);
    float rotation[9], up[3];
    axis_rotation(direction, float(X(angle) * 0.00277777784503996372222900390625f * 6.283185482025146484375f), rotation);
    runtime::matrices::transform_vectors(1, reinterpret_cast<const float *>(offset(behavior, 8)),
                                         rotation, up);
    std::memcpy(offset(state, 60), up, 12);
}
} // namespace nfs::game::world::camera
