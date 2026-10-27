#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::tar {
namespace {
Dword *allocate_target_texture(Dword width, Dword height, Dword depth) {
    auto *texture = static_cast<Dword *>(eagl::allocator(48, "EAGL::TAR new"));
    if (texture)
        construct(texture);
    auto *shared = reinterpret_cast<Dword *>(texture[9]);
    shared[1] = width;
    shared[2] = height;
    shared[3] = 1;
    shared[4] = depth;
    return texture;
}
void register_restore(Dword *texture, Dword kind) {
    auto *node = static_cast<Dword *>(eagl::allocator(16, "TARExtension::TARToRestore new"));
    if (node)
        node[0] = node[1] = node[2] = node[3] = 0;
    node[0] = reinterpret_cast<Dword>(texture);
    node[1] = kind;
    auto *head = backend::unresolved::texture_links;
    node[3] = reinterpret_cast<Dword>(head);
    if (head)
        head[2] = reinterpret_cast<Dword>(node);
    backend::unresolved::texture_links = node;
}
} // namespace

Dword *create_colour_target(Dword width, Dword height, Dword depth) {
    if (reinterpret_cast<std::uint8_t *>(backend::adapter_caps())[760] & 1)
        return nullptr;
    Dword *texture = allocate_target_texture(width, height, depth);
    Dword format = backend::texture_format(backend::selected_context(backend::selected_device()), depth);
    reinterpret_cast<Dword *>(texture[9])[5] = format;
    auto *shared = reinterpret_cast<Dword *>(texture[9]);
    backend::com(backend_device, 80, shared[1], shared[2], shared[3], Dword(1), shared[5], Dword(0),
                 shared + 6);
    register_restore(texture, 1);
    return texture;
}

Dword *create_depth_target(Dword *render_context, Dword width, Dword height, Dword depth) {
    if (reinterpret_cast<std::uint8_t *>(backend::adapter_caps())[760] & 1)
        return nullptr;
    Dword *texture = allocate_target_texture(width, height, depth);
    auto *shared = reinterpret_cast<Dword *>(texture[9]);
    int stencil = 0;
    backend::choose_depth(render_context, 0, 1, reinterpret_cast<int *>(shared + 4), &stencil,
                          shared + 5);
    Dword multisample;
    state::read_context_multisample_type(render_context, &multisample);
    shared = reinterpret_cast<Dword *>(texture[9]);
    backend::com(backend_device, 104, shared[1], shared[2], shared[5], multisample, shared + 7);
    register_restore(texture, 2);
    return texture;
}
} // namespace nfs::graphics::tar
