#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
int transition_index(float time, int count) {
    int index = channel_frame(time);
    if (index < 0)
        index = 0;
    if (int(Dword(count) - 1) <= index)
        index = int(Dword(count) - 2);
    return index;
}
void adjust_transition_clock(void *owner, unsigned weight_field, unsigned rate_field,
                             unsigned current_field, unsigned offset_field, unsigned first_rate,
                             const Extended &second_rate) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto previous_rate = X(at<float>(owner, rate_field));
    auto rate = (X(1) - at<float>(owner, weight_field)) / X(at<float>(owner, first_rate)) +
                X(at<float>(owner, weight_field)) / second_rate;
    at<float>(owner, rate_field) = float(rate);
    at<float>(owner, offset_field) =
        float((previous_rate / rate) *
                  (X(at<float>(owner, offset_field)) + at<float>(owner, current_field)) -
              at<float>(owner, current_field));
}
void *instantiate_clip(void *resource) {
    auto child = pending::create_runtime_animation(at<std::uint16_t>(resource, 0));
    world::invoke(child, 15, resource);
    return child;
}
void shift_clip_pair(void *owner, unsigned source_field, unsigned pair_field, int index,
                     int previous) {
    auto &first = at<Dword *>(owner, pair_field);
    auto &second = at<Dword *>(owner, pair_field + 4);
    if (index == int(Dword(previous) + 1)) {
        recycle_child_animation(first);
        first = second;
        second = static_cast<Dword *>(instantiate_clip(at<void **>(owner, source_field)[index + 1]));
    } else if (index == int(Dword(previous) - 1)) {
        recycle_child_animation(second);
        second = first;
        first = static_cast<Dword *>(instantiate_clip(at<void **>(owner, source_field)[index]));
    } else {
        if (first)
            recycle_child_animation(first);
        if (second)
            recycle_child_animation(second);
        first = static_cast<Dword *>(instantiate_clip(at<void **>(owner, source_field)[index]));
        second = static_cast<Dword *>(instantiate_clip(at<void **>(owner, source_field)[index + 1]));
    }
}
} // namespace

void __fastcall select_duration_transition(void *owner, void *, float position) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    int index = transition_index(position, at<int>(owner, 28));
    int previous = at<int>(owner, 32);
    at<float>(owner, 24) = float(X(position) - Extended::integer(index, format));
    if (index == previous) {
        adjust_transition_clock(owner, 24, 56, 60, 64, 40, X(at<float>(owner, 44)));
        return;
    }
    auto clips = at<void **>(owner, 12);
    if (index == int(Dword(previous) + 1))
        at<void *>(owner, 16) = at<void *>(owner, 20);
    else if (index == int(Dword(previous) - 1)) {
        at<void *>(owner, 20) = at<void *>(owner, 16);
        at<void *>(owner, 16) = clips[index];
    } else
        at<void *>(owner, 16) = clips[index];
    if (index != int(Dword(previous) - 1))
        at<void *>(owner, 20) = clips[index + 1];
    at<int>(owner, 32) = index;
    auto first = at<void **>(owner, 12)[index];
    at<float>(owner, 40) = float(X(1) / transition_clip_duration(first, nullptr));
    at<Dword>(owner, 48) = at<Dword>(first, 80);
    auto second = at<void **>(owner, 12)[at<int>(owner, 32) + 1];
    auto second_rate = X(1) / transition_clip_duration(second, nullptr);
    at<float>(owner, 44) = float(second_rate);
    at<Dword>(owner, 52) = at<Dword>(second, 80);
    adjust_transition_clock(owner, 24, 56, 60, 64, 40, second_rate);
}

void __fastcall select_frame_transition(void *owner, void *, float position) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    int index = transition_index(position, at<int>(owner, 44));
    int previous = at<int>(owner, 48);
    at<float>(owner, 40) = float(X(position) - Extended::integer(index, format));
    if (index == previous) {
        adjust_transition_clock(owner, 40, 72, 76, 80, 64, X(at<float>(owner, 68)));
        return;
    }
    shift_clip_pair(owner, 12, 24, index, previous);
    if (at<Dword>(owner, 20))
        shift_clip_pair(owner, 20, 32, index, at<int>(owner, 48));
    auto records = at<void **>(owner, 16);
    auto first = records[index], second = records[index + 1];
    unsigned first_start = at<std::uint16_t>(first, 6);
    if (!(at<Byte>(first, 8) & 1))
        first_start += at<Byte>(first, 10);
    at<float>(owner, 56) = float(first_start);
    at<float>(owner, 60) = float(at<std::uint16_t>(second, 6));
    if (!(at<Byte>(second, 8) & 1))
        at<float>(owner, 60) =
            float(Extended::integer(at<Byte>(second, 10), format) + at<float>(owner, 60));
    at<int>(owner, 48) = index;
    first = at<void **>(owner, 16)[index];
    at<float>(owner, 64) =
        float(Extended::integer(unsigned(at<Byte>(first, 11)) + at<Byte>(first, 10), format) * .5f);
    second = at<void **>(owner, 16)[index + 1];
    auto second_rate =
        Extended::integer(unsigned(at<Byte>(second, 11)) + at<Byte>(second, 10), format) * .5f;
    at<float>(owner, 68) = float(second_rate);
    adjust_transition_clock(owner, 40, 72, 76, 80, 64, second_rate);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
