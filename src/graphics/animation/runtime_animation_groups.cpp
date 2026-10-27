#include "runtime_animation_dispatch.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include <cstring>
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
Dword scaled_time(void *owner, Dword bits) {
    if (!at<Byte>(owner, 20))
        return bits;
    float value;
    std::memcpy(&value, &bits, 4);
    value *= float(at<Byte>(owner, 21));
    std::memcpy(&bits, &value, 4);
    return bits;
}
void ensure_group_children(void *owner) {
    void *resource = at<void *>(owner, 12);
    if (at<Dword>(owner, 16))
        return;
    at<Dword *>(owner, 16) = allocate_animation_buffer(Dword(at<std::uint16_t>(resource, 8)) * 4);
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index) {
        void *data = reinterpret_cast<void **>(offset(resource, 12))[index];
        void *animation = pending::create_runtime_animation(at<std::uint16_t>(data, 0));
        world::invoke(animation, 15, data);
        at<void **>(owner, 16)[index] = animation;
    }
}
void release_group_children(void *owner) {
    if (!at<Dword>(owner, 16))
        return;
    for (int index = int(at<std::uint16_t>(at<void *>(owner, 12), 8)) - 1; index >= 0; --index) {
        Dword *child = at<Dword **>(owner, 16)[index];
        world::invoke(child, 0, 0u);
        Dword &head = dependency::animation_state.scratch[child[2] & 65535];
        *child = head;
        head = reinterpret_cast<Dword>(child);
    }
    recycle_buffer(at<Dword *>(owner, 16));
}
} // namespace
// The new resource is stored before releasing old children.
void __fastcall bind_resource_format_15(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
    release_group_children(receiver);
    at<Dword>(receiver, 16) = 0;
}

void __fastcall advance_format_15(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    first_time = scaled_time(receiver, first_time);
    second_time = scaled_time(receiver, second_time);
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index)
        world::invoke(at<void **>(receiver, 16)[index], 3, first_time, second_time, output);
}

Dword __fastcall query_keyframe_format_15(void *receiver, void *, Dword time, Dword output, Dword mask,
                                         Dword context) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    time = scaled_time(receiver, time);
    output = scaled_time(receiver, output);
    Byte found = 0;
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index)
        found |=
            world::invoke<Byte>(at<void **>(receiver, 16)[index], 9, time, output, mask, context);
    return found;
}

Dword __fastcall sample_masked_format_15(void *receiver, void *, Dword time, Dword output, Dword mask) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    time = scaled_time(receiver, time);
    Byte found = 0;
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index)
        found |= world::invoke<Byte>(at<void **>(receiver, 16)[index], 6, time, output, mask);
    return found;
}

Dword __fastcall query_channel_format_15(void *receiver, void *, Dword time, Dword output) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    time = scaled_time(receiver, time);
    Byte found = 0;
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index)
        found |= world::invoke<Byte>(at<void **>(receiver, 16)[index], 10, time, output);
    return found;
}

Dword __fastcall query_sample_format_15(void *receiver, void *, Dword time, Dword output) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    time = scaled_time(receiver, time);
    Byte found = 0;
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index)
        found |= world::invoke<Byte>(at<void **>(receiver, 16)[index], 8, time, output);
    return found;
}

Dword __fastcall sample_channel_format_15(void *receiver, void *, Dword time, Dword output) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    time = scaled_time(receiver, time);
    Byte found = 0;
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index)
        found |= world::invoke<Byte>(at<void **>(receiver, 16)[index], 11, time, output);
    return found;
}
// This query uses existing children without lazy allocation.
Dword __fastcall query_event_format_15(void *receiver, void *, Dword time, Dword output, Dword mask) {
    output = scaled_time(receiver, output);
    Dword result = at<Dword>(receiver, 12);
    for (int index = int(at<std::uint16_t>(reinterpret_cast<void *>(result), 8)) - 1; index >= 0;
         --index) {
        result = world::invoke<Dword>(at<void **>(receiver, 16)[index], 12, time, output, mask);
        if (Byte(result)) {
            if (at<Byte>(receiver, 20)) {
                result = at<Byte>(receiver, 21);
                *reinterpret_cast<float *>(mask) /= float(result);
            }
            return (result & 4294967040u) | 1;
        }
    }
    return result & 4294967040u;
}
// The saved value query also does not allocate children.
Dword __fastcall sample_value_format_15(void *receiver, void *, Dword time, Dword output) {
    time = scaled_time(receiver, time);
    Byte found = 0;
    for (int index = int(at<std::uint16_t>(at<void *>(receiver, 12), 8)) - 1; index >= 0; --index)
        found |= world::invoke<Byte>(at<void **>(receiver, 16)[index], 7, time, output);
    return found;
}

Dword __fastcall read_owner_format_15(void *receiver, void *) {
    void *resource = at<void *>(receiver, 12);
    ensure_group_children(receiver);
    for (int index = int(at<std::uint16_t>(resource, 8)) - 1; index >= 0; --index) {
        Dword owner = world::invoke<Dword>(at<void **>(receiver, 16)[index], 13);
        if (owner)
            return owner;
    }
    return 0;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
