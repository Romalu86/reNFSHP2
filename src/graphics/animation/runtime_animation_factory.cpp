#include "runtime_animation_dispatch.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include <initializer_list>
namespace nfs::game::world::actor_resources::animation_runtime {
const std::uint16_t animation_sizes[22] = {20, 24, 20, 28, 44, 20, 128, 28, 128, 92, 24, 24,
                                           24, 24, 24, 24, 20, 48, 64,  48, 48,  48};

void reserve_animation_block(Dword format) {
    auto &state = dependency::animation_state;
    format &= 65535;
    *reinterpret_cast<Dword *>(state.cursor) = state.scratch[format];
    state.scratch[format] = reinterpret_cast<Dword>(state.cursor);
    state.cursor += animation_sizes[format];
}

void *allocate_animation(Dword format) {
    Dword &head = dependency::animation_state.scratch[format & 65535];
    if (!head)
        reserve_animation_block(format);
    auto *object = reinterpret_cast<Dword *>(head);
    head = *object;
    return object;
}

Dword *allocate_animation_buffer(Dword bytes) {
    auto &state = dependency::animation_state;
    Dword bucket = (bytes >> 4) & 255;
    if (Dword *head = reinterpret_cast<Dword *>(state.buckets[bucket])) {
        state.buckets[bucket] = *head;
        return head;
    }
    auto *header = reinterpret_cast<Dword *>(state.cursor);
    *header = bucket;
    state.cursor += bucket * 16 + 20;
    return header + 1;
}
void recycle_buffer(Dword *buffer) {
    Dword &head = dependency::animation_state.buckets[buffer[-1]];
    *buffer = head;
    head = reinterpret_cast<Dword>(buffer);
}

void * construct_resource_animation(void *animation) {
    at<Dword>(animation, 4) = at<Dword>(animation, 12) = 0;
    at<const Dword *>(animation, 0) = resource_animation_methods;
    return animation;
}

void destroy_resource_animation(void *animation) {
    at<const Dword *>(animation, 0) = animation_base_methods;
}

void * construct_indexed_animation(void *animation) {
    construct_resource_animation(animation);
    at<Dword>(animation, 16) = ~0u;
    at<Dword>(animation, 20) = 0;
    at<const Dword *>(animation, 0) = indexed_animation_methods;
    return animation;
}

void * construct_sampled_animation(void *animation) {
    construct_resource_animation(animation);
    at<Dword>(animation, 16) = ~0u;
    at<Dword>(animation, 20) = 0;
    at<const Dword *>(animation, 0) = sampled_animation_methods;
    return animation;
}

void * construct_animation_format_21(void *animation) {
    construct_resource_animation(animation);
    for (unsigned field : {20u, 24u, 32u, 36u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 16) = at<Dword>(animation, 28) = ~0u;
    at<const Dword *>(animation, 0) = format_21_methods;
    at<Dword>(animation, 8) = 21;
    return animation;
}

void * construct_animation_format_20(void *animation) {
    construct_resource_animation(animation);
    for (unsigned field : {20u, 24u, 32u, 36u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 16) = at<Dword>(animation, 28) = ~0u;
    at<const Dword *>(animation, 0) = format_20_methods;
    at<Dword>(animation, 8) = 20;
    return animation;
}

void * construct_animation_format_19(void *animation) {
    construct_resource_animation(animation);
    for (unsigned field : {16u, 20u, 32u, 36u, 40u, 44u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 24) = at<Dword>(animation, 28) = ~0u;
    at<const Dword *>(animation, 0) = format_19_methods;
    at<Dword>(animation, 8) = 19;
    return animation;
}

void * construct_animation_format_18(void *animation) {
    construct_resource_animation(animation);
    for (unsigned field : {16u, 20u, 32u, 36u, 44u, 48u, 52u, 56u, 60u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 24) = at<Dword>(animation, 28) = at<Dword>(animation, 40) = ~0u;
    at<const Dword *>(animation, 0) = format_18_methods;
    at<Dword>(animation, 8) = 18;
    return animation;
}

void * construct_animation_format_17(void *animation) {
    construct_resource_animation(animation);
    for (unsigned field : {16u, 20u, 32u, 36u, 40u, 44u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 24) = at<Dword>(animation, 28) = ~0u;
    at<const Dword *>(animation, 0) = format_17_methods;
    at<Dword>(animation, 8) = 17;
    return animation;
}

void * construct_animation_format_9(void *animation) {
    for (unsigned field : {4u, 12u, 24u, 28u, 60u, 64u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 32) = at<Dword>(animation, 68) = Dword(-100);
    at<Dword>(animation, 56) = 1065353216;
    at<Byte>(animation, 88) = 0;
    at<const Dword *>(animation, 0) = format_9_methods;
    at<Dword>(animation, 16) = at<Dword>(animation, 20) = 0;
    at<Dword>(animation, 8) = 9;
    return animation;
}

void * construct_animation_format_8(void *animation) {
    for (unsigned field : {4u, 12u, 16u, 20u, 40u, 44u, 76u, 80u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 48) = at<Dword>(animation, 84) = Dword(-100);
    at<Dword>(animation, 72) = 1065353216;
    at<Byte>(animation, 104) = 0;
    at<Dword>(animation, 124) = 0;
    at<const Dword *>(animation, 0) = format_8_methods;
    for (unsigned field : {24u, 28u, 32u, 36u})
        at<Dword>(animation, field) = 0;
    at<Dword>(animation, 8) = 8;
    return animation;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
namespace nfs::game::world::actor_resources::pending {

void *create_runtime_animation(Dword format) {
    using namespace animation_runtime;
    void *animation = allocate_animation(format);
    if (!animation)
        return nullptr;
    const Dword *methods;
    switch (format) {
    case 0:
    case 2:
        construct_resource_animation(animation);
        at<Byte>(animation, 16) = 1;
        methods = format == 0 ? format_0_methods : format_2_methods;
        break;
    case 1:
        construct_resource_animation(animation);
        at<Dword>(animation, 16) = at<Dword>(animation, 20) = 0;
        methods = format_1_methods;
        break;
    case 3:
    case 4:
    case 5:
        at<Dword>(animation, 4) = 0;
        methods = format == 3   ? format_3_methods
                  : format == 4 ? format_4_methods
                                : format_5_methods;
        break;
    case 6:
        at<Dword>(animation, 4) = at<Dword>(animation, 12) = 0;
        at<Dword>(animation, 40) = ~0u;
        at<const Dword *>(animation, 0) = format_6_methods;
        at<Dword>(animation, 20) = at<Dword>(animation, 16) = 0;
        at<Byte>(animation, 125) = at<Byte>(animation, 124) = 0;
        at<Dword>(animation, 8) = 6;
        return animation;
    case 7:
        at<Dword>(animation, 4) = at<Dword>(animation, 12) = at<Dword>(animation, 16) = 0;
        at<Byte>(animation, 24) = 0;
        at<Byte>(animation, 25) = 1;
        methods = format_7_methods;
        break;
    case 8:
        construct_animation_format_8(animation);
        return animation;
    case 9:
        construct_animation_format_9(animation);
        return animation;
    case 10:
    case 11:
        construct_indexed_animation(animation);
        methods = format == 10 ? format_10_methods : format_11_methods;
        break;
    case 12:
    case 13:
        construct_sampled_animation(animation);
        methods = format == 12 ? format_12_methods : format_13_methods;
        break;
    case 14:
        construct_resource_animation(animation);
        at<std::uint16_t>(animation, 16) = 0;
        at<Byte>(animation, 21) = 1;
        methods = format_14_methods;
        break;
    case 15:
        construct_resource_animation(animation);
        at<Dword>(animation, 16) = 0;
        at<Byte>(animation, 20) = at<Byte>(animation, 21) = 0;
        methods = format_15_methods;
        break;
    case 16:
        construct_resource_animation(animation);
        at<Dword>(animation, 16) = 0;
        methods = format_16_methods;
        break;
    case 17:
        construct_animation_format_17(animation);
        return animation;
    case 18:
        construct_animation_format_18(animation);
        return animation;
    case 19:
        construct_animation_format_19(animation);
        return animation;
    case 20:
        construct_animation_format_20(animation);
        return animation;
    case 21:
        construct_animation_format_21(animation);
        return animation;
    default:
        return animation;
    }
    at<const Dword *>(animation, 0) = methods;
    at<Dword>(animation, 8) = format;
    return animation;
}
} // namespace nfs::game::world::actor_resources::pending
