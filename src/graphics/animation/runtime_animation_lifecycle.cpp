#include "runtime_animation_dispatch.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include <initializer_list>
namespace nfs::game::world::actor_resources::animation_runtime {

void destroy_indexed_animation(void *animation) {
    at<const Dword *>(animation, 0) = indexed_animation_methods;
    if (auto *buffer = at<Dword *>(animation, 20))
        recycle_buffer(buffer);
    destroy_resource_animation(animation);
}

void destroy_sampled_animation(void *animation) {
    at<const Dword *>(animation, 0) = sampled_animation_methods;
    if (auto *buffer = at<Dword *>(animation, 20))
        recycle_buffer(buffer);
    destroy_resource_animation(animation);
}
void recycle_child_animation(Dword *child) {
    world::invoke(child, 0, 0u);
    const Dword descriptor = at<volatile Dword>(child, 8);
    Dword &head = dependency::animation_state.scratch[descriptor & 65535];
    *child = head;
    head = reinterpret_cast<Dword>(child);
}

void destroy_group_animation(void *animation) {
    at<const Dword *>(animation, 0) = format_15_methods;
    if (at<Dword>(animation, 16)) {
        for (int index = int(at<std::uint16_t>(at<void *>(animation, 12), 8)) - 1; index >= 0;
             --index)
            recycle_child_animation(at<Dword **>(animation, 16)[index]);
        recycle_buffer(at<Dword *>(animation, 16));
    }
    destroy_resource_animation(animation);
}

void destroy_animation_format_19(void *animation) {
    at<const Dword *>(animation, 0) = format_19_methods;
    if (auto *buffer = at<Dword *>(animation, 32)) {
        recycle_buffer(buffer);
        recycle_buffer(at<Dword *>(animation, 40));
        recycle_buffer(at<Dword *>(animation, 44));
    }
    destroy_resource_animation(animation);
}

void destroy_animation_format_18(void *animation) {
    at<const Dword *>(animation, 0) = format_18_methods;
    if (auto *buffer = at<Dword *>(animation, 32)) {
        recycle_buffer(buffer);
        recycle_buffer(at<Dword *>(animation, 16));
    }
    if (auto *buffer = at<Dword *>(animation, 44))
        recycle_buffer(buffer);
    destroy_resource_animation(animation);
}

void destroy_animation_format_17(void *animation) {
    at<const Dword *>(animation, 0) = format_17_methods;
    if (auto *buffer = at<Dword *>(animation, 32))
        recycle_buffer(buffer);
    destroy_resource_animation(animation);
}

dependency::AnimationBlock *animation_shared_block(Dword index) {
    return &dependency::animation_blocks[index];
}

void release_animation_shared_block(dependency::AnimationBlock *block) {
    block->reserved -= 1;
    if (block->address && int(block->reserved) < 1) {
        eagl::deallocator(block->address, block->bytes);
        block->address = nullptr;
    }
}

void destroy_animation_format_9(void *animation) {
    at<const Dword *>(animation, 0) = format_9_methods;
    if (at<Dword>(animation, 28))
        release_animation_shared_block(animation_shared_block(1));
    if (auto *buffer = at<Dword *>(animation, 12))
        recycle_buffer(buffer);
    at<const Dword *>(animation, 0) = animation_base_methods;
}

void destroy_animation_format_8(void *animation) {
    at<const Dword *>(animation, 0) = format_8_methods;
    for (unsigned field : {24u, 28u, 32u, 36u})
        if (auto *child = at<Dword *>(animation, field))
            recycle_child_animation(child);
    if (at<Dword>(animation, 44))
        release_animation_shared_block(animation_shared_block(0));
    for (unsigned field : {12u, 16u, 20u, 124u})
        if (auto *buffer = at<Dword *>(animation, field))
            recycle_buffer(buffer);
    at<const Dword *>(animation, 0) = animation_base_methods;
}

Dword __fastcall delete_format_0(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = format_0_methods;
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 20);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_1(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = format_1_methods;
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_2(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = format_2_methods;
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 20);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_16(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = format_16_methods;
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 20);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_5(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = animation_base_methods;
    if (flags & 1)
        eagl::deallocator(receiver, 20);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_6(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = animation_base_methods;
    if (flags & 1)
        eagl::deallocator(receiver, 128);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_7(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = animation_base_methods;
    if (flags & 1)
        eagl::deallocator(receiver, 28);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_3(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = animation_base_methods;
    if (flags & 1)
        eagl::deallocator(receiver, 28);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_4(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = animation_base_methods;
    if (flags & 1)
        eagl::deallocator(receiver, 44);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_animation_base(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = animation_base_methods;
    if (flags & 1)
        kernel::memory_free(receiver);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_10(void *receiver, void *, Dword flags) {
    destroy_indexed_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_11(void *receiver, void *, Dword flags) {
    destroy_indexed_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_indexed_animation(void *receiver, void *, Dword flags) {
    destroy_indexed_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_sampled_animation(void *receiver, void *, Dword flags) {
    destroy_sampled_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_12(void *receiver, void *, Dword flags) {
    destroy_sampled_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_13(void *receiver, void *, Dword flags) {
    destroy_sampled_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_14(void *receiver, void *, Dword flags) {
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_15(void *receiver, void *, Dword flags) {
    destroy_group_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 24);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_19(void *receiver, void *, Dword flags) {
    destroy_animation_format_19(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 48);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_18(void *receiver, void *, Dword flags) {
    destroy_animation_format_18(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 64);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_17(void *receiver, void *, Dword flags) {
    destroy_animation_format_17(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 48);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_9(void *receiver, void *, Dword flags) {
    destroy_animation_format_9(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 92);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_8(void *receiver, void *, Dword flags) {
    destroy_animation_format_8(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 128);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_resource_animation(void *receiver, void *, Dword flags) {
    destroy_resource_animation(receiver);
    if (flags & 1)
        kernel::memory_free(receiver);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_21(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = format_21_methods;
    if (at<Dword>(receiver, 24)) {
        recycle_buffer(at<Dword *>(receiver, 20));
        recycle_buffer(at<Dword *>(receiver, 32));
        recycle_buffer(at<Dword *>(receiver, 44));
    }
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 48);
    return reinterpret_cast<Dword>(receiver);
}

Dword __fastcall delete_format_20(void *receiver, void *, Dword flags) {
    at<const Dword *>(receiver, 0) = format_20_methods;
    if (at<Dword>(receiver, 24)) {
        recycle_buffer(at<Dword *>(receiver, 20));
        recycle_buffer(at<Dword *>(receiver, 32));
        recycle_buffer(at<Dword *>(receiver, 44));
    }
    destroy_resource_animation(receiver);
    if (flags & 1)
        eagl::deallocator(receiver, 48);
    return reinterpret_cast<Dword>(receiver);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
