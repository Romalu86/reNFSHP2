#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::dependency {
namespace pending {
// Constructs nested animations in reverse directory order.
void construct_animation_group(void *group) {
    for (int index = at<std::uint16_t>(group, 8) - 1; index >= 0; --index)
        construct_animation_object(reinterpret_cast<void **>(offset(group, 12))[index]);
}
} // namespace pending

void reset_animation_scratch() {
    animation_state.cursor = animation_state.pool;
    for (Dword &bucket : animation_state.buckets)
        bucket = 0;
    for (Dword &entry : animation_state.scratch)
        entry = 0;
}
// The saved flag clear writes only16 bits.
void initialize_animation_pool(Dword bytes) {
    animation_state.pool_bytes = bytes;
    animation_state.pool = static_cast<Byte *>(eagl::allocator(bytes, "EAGLAnim Memory Pool"));
    *reinterpret_cast<std::uint16_t *>(&animation_state.flags) = 0;
    reset_animation_scratch();
}

void release_animation_pool() {
    eagl::deallocator(animation_state.pool, animation_state.pool_bytes);
}
// The saved release does not clear the records.
void release_animation_blocks() {
    for (AnimationBlock &block : animation_blocks)
        if (block.address)
            eagl::deallocator(block.address, block.bytes);
}

void construct_animation_bank(void *bank) {
    for (int index = at<int>(bank, 4) - 1; index >= 0; --index)
        construct_animation_object(at<void **>(bank, 12)[index]);
}
// Confirmed single RET callback.
void destroy_animation_bank(void *) {
}

void construct_animation_object(void *object) {
    switch (at<std::uint16_t>(object, 0)) {
    case 0:
        pending::construct_uncompressed_animation(object);
        return;
    case 15:
        pending::construct_animation_group(object);
        return;
    default:
        return;
    }
}

void initialize_animation_factory(Dword bytes) {
    initialize_animation_pool(bytes);
    animation_factory = eagl::allocator(1, "EAGLAnim::FnAnimFactory new");
    eagl::callbacks_register(&eagl::constructors, "AnimationBank",
                             reinterpret_cast<Dword>(construct_animation_bank),
                             reinterpret_cast<Dword>(destroy_animation_bank));
}

void shutdown_animation_factory() {
    eagl::callbacks_remove(&eagl::constructors, "AnimationBank");
    release_animation_blocks();
    if (animation_factory)
        eagl::deallocator(animation_factory, 1);
    release_animation_pool();
}
} // namespace nfs::game::world::dependency
