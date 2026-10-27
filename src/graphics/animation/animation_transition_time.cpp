#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
int cycle_index(float time, float start, float end) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto range = X(end) - start;
    if (time < start)
        return channel_frame((X(start) - time) / range);
    if (std::isnan(time) || std::isnan(end) || time < end)
        return 0;
    return int(Dword(channel_frame((X(time) - end) / range)) + 1);
}
Extended wrapped_time(float time, float start, float end) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    float range = float(X(end) - start);
    if (time < start) {
        auto distance = X(start) - time;
        float stored_distance = float(distance);
        int cycles = channel_frame(distance / X(range));
        return X(end) - (X(stored_distance) - Extended::integer(cycles, format) * range);
    }
    if (!std::isnan(time) && !std::isnan(end) && end <= time) {
        auto distance = X(time) - end;
        float stored_distance = float(distance);
        int cycles = channel_frame(distance / X(range));
        return X(stored_distance) - Extended::integer(cycles, format) * range + start;
    }
    return X(time);
}
} // namespace

int __stdcall duration_cycle_index(float time, float start, float end) {
    return cycle_index(time, start, end);
}

int __stdcall frame_cycle_index(float time, float start, float end) {
    return cycle_index(time, start, end);
}
// Extended return value has an explicit C++ adapter.
Extended __stdcall wrap_duration_time(float time, float start, float end) {
    return wrapped_time(time, start, end);
}

Extended __stdcall wrap_frame_time(float time, float start, float end) {
    return wrapped_time(time, start, end);
}

Extended __fastcall transition_clip_duration(void *clip, void *) {
    return Extended(at<float>(clip, 72), kernel::numeric::current_format());
}
} // namespace nfs::game::world::actor_resources::animation_runtime
