#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
Dword duration_turn_updates;
namespace {
bool initialize_turn(void *owner, unsigned initialized_field, unsigned cycle_field,
                     unsigned rotation_field) {
    if (at<Byte>(owner, initialized_field))
        return false;
    at<int>(owner, cycle_field) = -1;
    at<Dword>(owner, rotation_field) = at<Dword>(owner, rotation_field + 4) =
        at<Dword>(owner, rotation_field + 8) = 0;
    at<float>(owner, rotation_field + 12) = 1;
    at<Byte>(owner, initialized_field) = 1;
    return true;
}
void accumulate_turn(void *owner, unsigned rotation_field, unsigned cycle_field, int cycle,
                     const float *delta, bool duration) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto rotation = reinterpret_cast<float *>(offset(owner, rotation_field));
    float previous[4];
    std::memcpy(previous, rotation, 16);
    auto base_x = X(delta[3]) * previous[0] - X(delta[2]) * previous[1];
    float x = duration ? float((base_x + X(delta[0]) * previous[3]) + X(delta[1]) * previous[2])
                       : float((base_x + X(delta[1]) * previous[2]) + X(delta[0]) * previous[3]);
    float y = float(
        ((X(delta[3]) * previous[1] + X(delta[2]) * previous[0]) - X(delta[0]) * previous[2]) +
        X(delta[1]) * previous[3]);
    auto base_z = X(delta[2]) * previous[3] - X(delta[1]) * previous[0];
    float z = duration ? float((base_z + X(delta[0]) * previous[1]) + X(delta[3]) * previous[2])
                       : float((base_z + X(delta[3]) * previous[2]) + X(delta[0]) * previous[1]);
    auto dot =
        duration
            ? (X(delta[1]) * previous[1] + X(delta[0]) * previous[0]) + X(delta[2]) * previous[2]
            : (X(delta[2]) * previous[2] + X(delta[0]) * previous[0]) + X(delta[1]) * previous[1];
    rotation[0] = x;
    if (duration)
        at<int>(owner, cycle_field) = cycle;
    rotation[1] = y;
    rotation[2] = z;
    if (!duration)
        at<int>(owner, cycle_field) = cycle;
    rotation[3] = float(X(delta[3]) * previous[3] - dot);
}
} // namespace

void __fastcall advance_duration_turn(void *owner, void *, int cycle) {
    if (initialize_turn(owner, 88, 68, 72) || at<int>(owner, 68) == cycle)
        return;
    float first[2], second[2], turn[4];
    duration_start_direction(owner, nullptr, first);
    duration_end_direction(owner, nullptr, second);
    duration_turn_between_directions(first, second, turn);
    if (int(Dword(at<int>(owner, 68)) - 1) == cycle)
        turn[1] = float(Extended(turn[1], kernel::numeric::current_format()) * -1.0f);
    accumulate_turn(owner, 72, 68, cycle, turn, true);
    ++duration_turn_updates; // The saved release-console call itself is a complete RET leaf.
}

void __fastcall advance_frame_turn(void *owner, void *, int cycle) {
    if (initialize_turn(owner, 104, 84, 88) || at<int>(owner, 84) == cycle)
        return;
    float first[2], second[2], turn[4];
    sample_frame_direction(owner, nullptr, 0, 0, first);
    auto frames = at<void **>(owner, 16);
    int index = at<int>(owner, 48);
    sample_frame_direction(owner, nullptr, float(int(at<std::uint16_t>(frames[index], 4)) - 1),
                           float(int(at<std::uint16_t>(frames[index + 1], 4)) - 1), second);
    frame_turn_between_directions(first, second, turn);
    if (int(Dword(at<int>(owner, 84)) - 1) == cycle)
        turn[1] = float(Extended(turn[1], kernel::numeric::current_format()) * -1.0f);
    accumulate_turn(owner, 88, 84, cycle, turn, false);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
