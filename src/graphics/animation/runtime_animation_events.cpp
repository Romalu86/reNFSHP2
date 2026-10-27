#include "runtime_animation_dispatch.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
float time_value(Dword word) {
    float value;
    std::memcpy(&value, &word, 4);
    return value;
}
struct AnimationEvent {
    Dword identifier;
    float time;
    Dword reserved;
    float value;
};
static_assert(sizeof(AnimationEvent) == 16);
} // namespace

void dispatch_animation_events(void *resource, float previous, float current, int *cursor,
                               float *previous_time, void **targets, Dword context) {
    int count = at<int>(resource, 4);
    auto *events = reinterpret_cast<AnimationEvent *>(offset(resource, 8));
    int index = *cursor;
    if (previous < *previous_time) {
        while (index >= 0) {
            if (events[index].time <= previous)
                break;
            --index;
        }
        index += 1;
        *cursor = index;
    }
    while (index < count) {
        if (previous < events[index].time)
            break;
        ++index;
    }
    *cursor = index;
    bool unequal = previous != current;
    auto deliver = [&](int event_index) {
        AnimationEvent *event = events + event_index;
        if (void *target = targets[event->identifier])
            world::invoke(target, 0, current, event, context);
    };
    if (unequal) {
        if (current < previous) {
            for (; index < count; ++index)
                deliver(index);
            index = *cursor = 0;
        }
    } else {
        if (index >= count)
            index = *cursor = count - 1;
        while (index >= 0) {
            if (events[index].time < current)
                break;
            --index;
        }
        index += 1;
        *cursor = index;
    }
    while (index < count && events[index].time <= current) {
        deliver(index);
        ++index;
    }
    *cursor = index < count ? index : count - 1;
    *previous_time = current;
}

Dword __fastcall query_keyframe_format_1(void *receiver, void *, Dword time, Dword output, Dword mask,
                                        Dword context) {
    dispatch_animation_events(at<void *>(receiver, 12), time_value(time), time_value(output),
                              reinterpret_cast<int *>(offset(receiver, 16)),
                              reinterpret_cast<float *>(offset(receiver, 20)),
                              reinterpret_cast<void **>(mask), context);
    return 1;
}

void __fastcall bind_resource_format_10(void *receiver, void *, Dword resource) {
    void *data = reinterpret_cast<void *>(resource);
    Dword *buffer = at<Dword *>(receiver, 20);
    Dword count = *at<std::uint16_t *>(data, 4);
    if (!buffer || count >= *at<std::uint16_t *>(at<void *>(receiver, 12), 4)) {
        if (buffer)
            recycle_buffer(buffer);
        at<Dword *>(receiver, 20) = allocate_animation_buffer(count * 4);
    }
    at<Dword>(receiver, 12) = resource;
    at<Dword>(receiver, 16) = ~0u;
}

void __fastcall bind_resource_format_12(void *receiver, void *, Dword resource) {
    void *data = reinterpret_cast<void *>(resource);
    Dword *buffer = at<Dword *>(receiver, 20);
    Dword count = *at<std::uint16_t *>(data, 4);
    if (!buffer || count >= *at<std::uint16_t *>(at<void *>(receiver, 12), 4)) {
        if (buffer)
            recycle_buffer(buffer);
        at<Dword *>(receiver, 20) = allocate_animation_buffer(count * 4);
    }
    at<Dword>(receiver, 12) = resource;
    at<Dword>(receiver, 16) = ~0u;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
