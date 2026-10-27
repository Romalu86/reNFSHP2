#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {

void blend_transition_poses(int count, float weight, const float *first, const float *second,
                            float *out, const Dword *mask) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    for (int bone = 0; bone < count; ++bone, first += 12, second += 12, out += 12) {
        if (mask && !(mask[unsigned(bone) >> 5] & (Dword(1) << (bone & 31))))
            continue;
        auto dot =
            !mask
                ? ((X(second[7]) * first[7] + X(second[6]) * first[6]) + X(first[5]) * second[5]) +
                      X(first[4]) * second[4]
                : ((X(first[7]) * second[7] + X(first[6]) * second[6]) + X(second[5]) * first[5]) +
                      X(second[4]) * first[4];
        for (unsigned i = 4; i < 8; ++i) {
            auto sum = ((!mask && i >= 6) || (mask && i == 4)) ? X(second[i]) + first[i]
                                                               : X(first[i]) + second[i];
            out[i] = float(!(X(0) < dot) ? X(first[i]) - sum * weight
                                         : (X(second[i]) - first[i]) * weight + first[i]);
        }
        auto square = !mask ? ((X(out[4]) * out[4] + X(out[5]) * out[5]) + X(out[6]) * out[6]) +
                                  X(out[7]) * out[7]
                            : ((X(out[6]) * out[6] + X(out[7]) * out[7]) + X(out[4]) * out[4]) +
                                  X(out[5]) * out[5];
        auto inverse = X(1) / square.square_root();
        for (unsigned i = 4; i < 8; ++i)
            out[i] = float(inverse * out[i]);
        blend_animation_translation(weight, first + 8, second + 8, out + 8);
    }
}
namespace {
struct TransitionTimes {
    float first, second;
    int cycle;
};
TransitionTimes duration_times(void *owner, Dword time_word, bool query) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    at<Dword>(owner, 60) = time_word;
    if (!at<Dword>(owner, 16))
        select_duration_transition(owner, nullptr, 0);
    float shifted = float(X(channel_float(time_word)) + at<float>(owner, 64));
    int cycle = duration_cycle_index(shifted, 0, float(X(2) / X(at<float>(owner, 56))));
    float first =
        float(X(at<float>(owner, query ? 56 : 40)) * at<float>(owner, query ? 40 : 56) * shifted);
    float second = query ? float(X(at<float>(owner, 56)) * at<float>(owner, 44) * shifted)
                         : float(X(shifted) * at<float>(owner, 56) * at<float>(owner, 44));
    first =
        float(wrap_duration_time(first, 0, float(X(at<float>(owner, 40)) + at<float>(owner, 40))) -
              at<float>(owner, 48));
    second =
        float(wrap_duration_time(second, 0, float(X(at<float>(owner, 44)) + at<float>(owner, 44))) -
              at<float>(owner, 52));
    return {first, second, cycle};
}
TransitionTimes frame_times(void *owner, Dword time_word) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto shifted = X(channel_float(time_word)) + at<float>(owner, 80);
    auto frames = at<void **>(owner, 16);
    int index = at<int>(owner, 48);
    float first =
        float(X(at<float>(owner, 64)) * at<float>(owner, 72) * shifted + at<float>(owner, 56));
    float second =
        float(X(at<float>(owner, 68)) * at<float>(owner, 72) * shifted + at<float>(owner, 60));
    float first_end = float(int(at<std::uint16_t>(frames[index], 4)) - 1);
    int cycle = frame_cycle_index(first, 0, first_end);
    first = float(wrap_frame_time(
        first, 0,
        float(int(at<std::uint16_t>(at<void **>(owner, 16)[at<int>(owner, 48)], 4)) - 1)));
    second = float(wrap_frame_time(
        second, 0,
        float(int(at<std::uint16_t>(at<void **>(owner, 16)[at<int>(owner, 48) + 1], 4)) - 1)));
    return {first, second, cycle};
}
} // namespace
// The saved method ignores its final stack word.
Dword __fastcall sample_duration_blended_pose(void *owner, void *, Dword time, Dword output, Dword) {
    auto times = duration_times(owner, time, false);
    auto pose = reinterpret_cast<Dword *>(output);
    vehicle_render::restore_skeleton_pose(at<void *>(owner, 36), pose, nullptr);
    if (!Byte(world::invoke<Dword>(at<void *>(owner, 16), 6, channel_word(times.first), pose, 0u)))
        return 0;
    if (at<float>(owner, 24) != 0) {
        auto scratch = static_cast<Dword *>(animation_shared_block(1)->address);
        vehicle_render::restore_skeleton_pose(at<void *>(owner, 36), scratch, nullptr);
        if (!Byte(world::invoke<Dword>(at<void *>(owner, 20), 6, channel_word(times.second), scratch,
                                      0u)))
            return 0;
        blend_transition_poses(at<int>(at<void *>(owner, 36), 8), at<float>(owner, 24),
                               reinterpret_cast<float *>(pose), reinterpret_cast<float *>(scratch),
                               reinterpret_cast<float *>(pose), nullptr);
    }
    advance_duration_turn(owner, nullptr, times.cycle);
    apply_duration_turn_to_pose(owner, nullptr, reinterpret_cast<float *>(pose));
    return 1;
}

Dword __fastcall sample_duration_blended_displacement(void *owner, void *, Dword time, Dword output) {
    auto times = duration_times(owner, time, true);
    auto out = reinterpret_cast<float *>(output);
    if (!Byte(sample_duration_displacement(owner, nullptr, times.first, times.second, out)))
        return 0;
    advance_duration_turn(owner, nullptr, times.cycle);
    apply_duration_turn_to_displacement(owner, nullptr, out);
    return 1;
}
// The saved method ignores its final stack word.
Dword __fastcall sample_frame_blended_pose(void *owner, void *, Dword time, Dword output, Dword) {
    at<Dword>(owner, 76) = time;
    if (!at<Dword>(owner, 24))
        select_frame_transition(owner, nullptr, 0);
    auto times = frame_times(owner, time);
    auto pose = reinterpret_cast<Dword *>(output);
    vehicle_render::restore_skeleton_pose(at<void *>(owner, 52), pose, nullptr);
    if (!Byte(world::invoke<Dword>(at<void *>(owner, 24), 6, channel_word(times.first), pose, 0u)))
        return 0;
    if (at<float>(owner, 40) != 0) {
        auto scratch = static_cast<Dword *>(animation_shared_block(0)->address);
        vehicle_render::restore_skeleton_pose(at<void *>(owner, 52), scratch, nullptr);
        if (!Byte(world::invoke<Dword>(at<void *>(owner, 28), 6, channel_word(times.second), scratch,
                                      0u)))
            return 0;
        blend_transition_poses(at<int>(at<void *>(owner, 52), 8), at<float>(owner, 40),
                               reinterpret_cast<float *>(pose), reinterpret_cast<float *>(scratch),
                               reinterpret_cast<float *>(pose), nullptr);
    }
    advance_frame_turn(owner, nullptr, times.cycle);
    apply_frame_turn_to_pose(owner, nullptr, reinterpret_cast<float *>(pose));
    return 1;
}

Dword __fastcall sample_frame_blended_displacement(void *owner, void *, Dword time, Dword output) {
    at<Dword>(owner, 76) = time;
    if (!at<Dword>(owner, 20))
        return 0;
    if (!at<Dword>(owner, 32))
        select_frame_transition(owner, nullptr, 0);
    auto times = frame_times(owner, time);
    auto out = reinterpret_cast<float *>(output);
    if (!Byte(sample_frame_displacement(owner, nullptr, times.first, times.second, out)))
        return 0;
    advance_frame_turn(owner, nullptr, times.cycle);
    apply_frame_turn_to_displacement(owner, nullptr, out);
    return 1;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
