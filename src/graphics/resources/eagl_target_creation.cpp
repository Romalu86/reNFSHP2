#include "nfs/graphics/resources/eagl_assets.hpp"
#include <initializer_list>
namespace nfs::graphics::targets {
namespace {
void remove_binding(Dword *&head, Dword *owner) {
    for (Dword *node = head; node; node = reinterpret_cast<Dword *>(node[3])) {
        if (node[0] != reinterpret_cast<Dword>(owner))
            continue;
        if (node[2])
            reinterpret_cast<Dword *>(node[2])[3] = node[3];
        if (node[3])
            reinterpret_cast<Dword *>(node[3])[2] = node[2];
        if (head == node)
            head = reinterpret_cast<Dword *>(node[3]);
        node[0] = node[1] = node[2] = node[3] = 0;
        eagl::deallocator(node, 16);
        break;
    }
}
void add_binding(Dword *&head, Dword *owner, Dword *texture, const char *label) {
    auto *node = static_cast<Dword *>(eagl::allocator(16, label));
    if (node)
        node[0] = node[1] = node[2] = node[3] = 0;
    node[1] = reinterpret_cast<Dword>(texture);
    node[0] = reinterpret_cast<Dword>(owner);
    node[3] = reinterpret_cast<Dword>(head);
    if (head)
        head[2] = reinterpret_cast<Dword>(node);
    head = node;
}
} // namespace

void set_owner(Dword *object, Dword *owner) {
    object[0] = reinterpret_cast<Dword>(owner);
}

void clear_colour_state(Dword *state) {
    state[0] = state[1] = state[2] = state[3] = 0;
}

void clear_depth_state(Dword *state) {
    state[0] = state[1] = state[2] = 0;
}

Dword *construct_state(Dword *state, Dword *owner) {
    state[0] = reinterpret_cast<Dword>(owner);
    clear_colour_state(state + 1);
    clear_depth_state(state + 5);
    for (unsigned index : {8u, 9u, 10u, 11u, 12u, 13u, 32u})
        state[index] = 0;
    return state;
}

Dword *construct(Dword *target, Dword *device_state) {
    set_owner(target, target);
    construct_state(target + 1, target);
    target[208 / 4] = reinterpret_cast<Dword>(device_state);
    return target;
}
// The saved allocation failure still reaches the low-address store.
Dword *create(Device *owner_device) {
    auto *target = static_cast<Dword *>(eagl::allocator(212, "TextureRenderContext new"));
    if (target)
        construct(target, owner_device->limits);
    *reinterpret_cast<volatile Dword *>(reinterpret_cast<std::uintptr_t>(target) + 48) =
        owner_device->limits[12];
    owner_device->limits[12] = reinterpret_cast<Dword>(target);
    return target;
}

Dword *create_viewport(Dword *target) {
    auto *viewport = static_cast<Dword *>(eagl::allocator(372, "EAGL::Viewport new"));
    if (viewport)
        viewports::construct_target(viewport, target);
    *reinterpret_cast<volatile Dword *>(reinterpret_cast<std::uintptr_t>(viewport) + 364) =
        target[10];
    target[10] = reinterpret_cast<Dword>(viewport);
    return viewport;
}

Dword bind_textures(Dword *target, Dword *colour, Dword *depth) {
    if (target[13]) {
        remove_binding(first_links, target + 1);
        if (target[13])
            backend::com(reinterpret_cast<Dword *>(target[13]), 8);
        target[13] = 0;
        remove_binding(second_links, target + 1);
    }
    auto *colour_data = reinterpret_cast<Dword *>(colour[9]);
    auto *depth_data = depth ? reinterpret_cast<Dword *>(depth[9]) : nullptr;
    target[5] = colour_data[2];
    target[4] = colour_data[1];
    backend::com(reinterpret_cast<Dword *>(colour_data[6]), 60, Dword(0), target + 13);
    target[14] = 0;
    target[7] = reinterpret_cast<Dword *>(colour[9])[4];
    if (depth) {
        target[14] = depth_data[7];
        target[8] = reinterpret_cast<Dword *>(depth[9])[4];
    }
    add_binding(first_links, target + 1, colour,
                "TextureRenderContextPrivate::FrontBufferToRestore new");
    add_binding(second_links, target + 1, depth,
                "TextureRenderContextPrivate::ZBufferToRestore new");
    return 1;
}
} // namespace nfs::graphics::targets
