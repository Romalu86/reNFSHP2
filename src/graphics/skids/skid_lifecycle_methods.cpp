#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <initializer_list>

namespace nfs::game::runtime::skids::lifecycle::pending {

Dword *construct_render_method(Dword *method, Dword *parent) {
    method[7] = 1;
    method[8] = reinterpret_cast<Dword>(parent);
    if (parent[3])
        graphics::methods::initialize_variant(method);
    else {
        method[4] =
            reinterpret_cast<Dword>(graphics::backend::unresolved::secondary_methods);
        graphics::backend::unresolved::secondary_methods = method;
    }
    return method;
}

void destroy_render_method(Dword *method) {
    if (method[7] && method[0])
        graphics::methods::rebuild(method);
}
} // namespace nfs::game::runtime::skids::lifecycle::pending
namespace nfs::game::runtime::skids::lifecycle {

Dword *construct_array(Dword *array) {
    graphics::immediate::model_builder_construct(array);
    for (unsigned offset : {124, 80, 108, 112, 116, 120, 100, 104})
        at<Dword>(array, offset) = 0;
    return array;
}

void destroy_array(Dword *array) {
    if (!(at<Byte>(array, 80) & 8))
        for (unsigned offset : {120, 116, 112, 108})
            if (void *data = at<void *>(array, offset))
                kernel::memory_free(data);
    for (unsigned offset : {104, 100, 124})
        if (void *data = at<void *>(array, offset))
            kernel::memory_free(data);
    graphics::immediate::model_builder_destroy(array);
}

void unload_level(Dword *owner) {
    if (Dword *array = reinterpret_cast<Dword *>(owner[3])) {
        destroy_array(array);
        eagl::deallocator(array, 128);
    }
    owner[3] = 0;
    if (Dword *primitive = reinterpret_cast<Dword *>(owner[4])) {
        if (Dword *method = reinterpret_cast<Dword *>(primitive[0])) {
            pending::destroy_render_method(method);
            eagl::deallocator(method, 44);
        }
        eagl::deallocator(primitive, 84);
    }
    owner[4] = 0;
    if (Dword *texture = reinterpret_cast<Dword *>(owner[96 / 4])) {
        graphics::tar::destroy(texture);
        eagl::deallocator(texture, 48);
    }
    owner[96 / 4] = 0;
    if (void *image = reinterpret_cast<void *>(owner[92 / 4]))
        kernel::memory_free(image);
    owner[92 / 4] = 0;
}
} // namespace nfs::game::runtime::skids::lifecycle
