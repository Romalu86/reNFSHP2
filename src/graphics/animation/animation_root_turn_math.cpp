#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
Extended vector_length(const float *vector) {
    auto format = kernel::numeric::current_format();
    auto x = Extended(vector[0], format), y = Extended(vector[1], format);
    return (x * x + y * y).square_root();
}
void direction_turn(const float *first, const float *second, float *rotation) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto dot = X(first[1]) * second[1] + X(first[0]) * second[0];
    rotation[0] = rotation[2] = 0;
    auto first_length = vector_length(first), second_length = vector_length(second);
    // FDIVP computes dot/(lengthA*lengthB); the pseudocode inverted this ratio.
    auto half_cosine = (dot / (first_length * second_length) + 1.0f) * .5f;
    float sine = float((X(1) - half_cosine).square_root());
    rotation[1] = sine;
    rotation[3] = float(half_cosine.square_root());
    if (X(0) < X(first[0]) * second[1] - X(first[1]) * second[0])
        rotation[1] = float(X(sine) * -1.0f);
}
void apply_pose_turn(const float *turn, float *pose) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    float p[4];
    std::memcpy(p, pose + 4, 16);
    float result[4];
    result[0] =
        float(((X(p[0]) * turn[3] - X(turn[2]) * p[1]) + X(turn[0]) * p[3]) + X(turn[1]) * p[2]);
    result[1] =
        float(((X(turn[3]) * p[1] + X(p[0]) * turn[2]) - X(turn[0]) * p[2]) + X(turn[1]) * p[3]);
    result[2] =
        float(((X(turn[3]) * p[2] + X(turn[2]) * p[3]) - X(p[0]) * turn[1]) + X(turn[0]) * p[1]);
    result[3] =
        float(X(turn[3]) * p[3] - ((X(turn[1]) * p[1] + X(turn[2]) * p[2]) + X(p[0]) * turn[0]));
    std::memcpy(pose + 4, result, 16);
}
void apply_displacement_turn(const float *turn, float *displacement) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    float twice_x = float(X(turn[0]) + turn[0]), twice_z = float(X(turn[2]) + turn[2]);
    auto twice_y = X(turn[1]) + turn[1];
    float yw = float(twice_y * turn[3]), zx = float(X(twice_z) * turn[0]),
          yy = float(twice_y * turn[1]);
    float x = displacement[0], y = displacement[1];
    auto horizontal_zero = (twice_y * turn[0] - X(twice_z) * turn[3]) * 0.0f;
    auto horizontal = horizontal_zero + (X(1) - (X(twice_z) * turn[2] + yy)) * x;
    horizontal = horizontal + (X(zx) + yw) * y;
    auto vertical_zero = (X(twice_z) * turn[1] + X(twice_x) * turn[3]) * 0.0f;
    auto vertical = vertical_zero + (X(1) - (X(twice_x) * turn[0] + yy)) * y;
    vertical = vertical + (X(zx) - yw) * x;
    displacement[0] = float(horizontal);
    displacement[1] = float(vertical);
}
} // namespace

Extended duration_displacement_length(const float *vector) {
    return vector_length(vector);
}

Extended frame_displacement_length(const float *vector) {
    return vector_length(vector);
}

void duration_turn_between_directions(const float *first, const float *second, float *rotation) {
    direction_turn(first, second, rotation);
}

void frame_turn_between_directions(const float *first, const float *second, float *rotation) {
    direction_turn(first, second, rotation);
}

void __fastcall apply_duration_turn_to_pose(void *owner, void *, float *pose) {
    apply_pose_turn(reinterpret_cast<const float *>(offset(owner, 72)), pose);
}

void __fastcall apply_frame_turn_to_pose(void *owner, void *, float *pose) {
    apply_pose_turn(reinterpret_cast<const float *>(offset(owner, 88)), pose);
}

void __fastcall apply_duration_turn_to_displacement(void *owner, void *, float *displacement) {
    apply_displacement_turn(reinterpret_cast<const float *>(offset(owner, 72)), displacement);
}

void __fastcall apply_frame_turn_to_displacement(void *owner, void *, float *displacement) {
    apply_displacement_turn(reinterpret_cast<const float *>(offset(owner, 88)), displacement);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
