#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
float wrapped_time(void *owner, float time) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float lower = at<float>(owner, 12), upper = at<float>(owner, 16), period = at<float>(owner, 20);
    bool below = time < lower;
    if (!below && !(upper < time))
        return time;
    auto difference = X(time) - (below ? lower : upper);
    float stored_difference = float(difference);
    auto ratio = difference / X(period);
    bool negative = ratio < X(0) || std::isnan(float(ratio));
    float adjusted = float(negative ? ratio + 0.499999f : ratio - 0.499999f);
    int turns = channel_integer(X(adjusted));
    auto remainder = X(stored_difference) - Extended::integer(turns, format) * period;
    return float(below ? X(upper) - remainder : remainder + lower);
}
} // namespace

void __fastcall advance_wrapped_animation(void *receiver, void *, Dword first_time, Dword second_time,
                                          Dword output) {
    float second = wrapped_time(receiver, channel_float(second_time));
    float first = wrapped_time(receiver, channel_float(first_time));
    world::invoke(at<void *>(receiver, 24), 3, first, second, output);
}

Dword __fastcall query_wrapped_keyframe(void *receiver, void *, Dword time, Dword output, Dword mask,
                                       Dword context) {
    float second = wrapped_time(receiver, channel_float(output));
    float first = wrapped_time(receiver, channel_float(time));
    return world::invoke<Dword>(at<void *>(receiver, 24), 9, first, second, mask, context);
}
// Terminal virtual jump forwards the other two stack words.
Dword __fastcall sample_wrapped_pose(void *receiver, void *, Dword time, Dword output, Dword mask) {
    float sample = wrapped_time(receiver, channel_float(time));
    return world::invoke<Dword>(at<void *>(receiver, 24), 6, sample, output, mask);
}

Dword __fastcall sample_wrapped_value(void *receiver, void *, Dword time, Dword output) {
    float sample = wrapped_time(receiver, channel_float(time));
    return world::invoke<Dword>(at<void *>(receiver, 24), 7, sample, output);
}

void __fastcall advance_transition_animation(void *receiver, void *, Dword first_time,
                                             Dword second_time, Dword output) {
    auto format = kernel::numeric::current_format();
    float previous = channel_float(first_time), current = channel_float(second_time);
    auto send = [&](unsigned child, unsigned offset_time) {
        float second = float(Extended(current, format) - at<float>(receiver, offset_time));
        float first = float(Extended(previous, format) - at<float>(receiver, offset_time));
        world::invoke(at<void *>(receiver, child), 3, first, second, output);
    };
    if (current <= at<float>(receiver, 28)) {
        send(12, 20);
        return;
    }
    if (current < at<float>(receiver, 32) || std::isnan(current) ||
        std::isnan(at<float>(receiver, 32))) {
        Dword policy = at<Dword>(receiver, 40);
        if (!policy) {
            send(12, 20);
            return;
        }
        if (policy != 1) {
            send(12, 20);
            send(16, 24);
            return;
        }
    }
    send(16, 24);
}

void __fastcall advance_mirrored_animation(void *receiver, void *, Dword first_time,
                                           Dword second_time, Dword output) {
    void *child = at<void *>(receiver, 12);
    if (at<Byte>(receiver, 25)) {
        world::invoke(child, 3, first_time, second_time, at<Dword>(receiver, 20));
        mirror_animation_pose(at<void *>(receiver, 16), at<void *>(receiver, 20),
                              reinterpret_cast<void *>(output), at<Byte>(receiver, 24), nullptr);
    } else
        world::invoke(child, 3, first_time, second_time, output);
}

Dword __fastcall sample_mirrored_pose(void *receiver, void *, Dword time, Dword output, Dword mask) {
    void *child = at<void *>(receiver, 12);
    if (!at<Byte>(receiver, 25))
        return world::invoke<Dword>(child, 6, time, output, mask);
    Byte sampled = world::invoke<Byte>(child, 6, time, at<Dword>(receiver, 20), mask);
    if (sampled)
        mirror_animation_pose(at<void *>(receiver, 16), at<void *>(receiver, 20),
                              reinterpret_cast<void *>(output), at<Byte>(receiver, 24),
                              reinterpret_cast<Dword *>(mask));
    return sampled;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
