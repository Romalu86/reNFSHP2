#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
namespace nfs::game::runtime::skids::lifecycle::pending {
// Allocation flags retain the saved bit tests, including paired buffers.
void allocate_vertices(Dword *array, int count, Dword topology, Dword *primitive, Dword *render_state,
                       Dword *texture, Dword flags, const char *label, Dword pool) {
    at<Dword>(array, 76) = Dword(count);
    at<Dword>(array, 80) = flags & 65535;
    auto allocate = [&](unsigned offset, Dword stride) {
        at<void *>(array, offset) = kernel::memory_allocate(label, int(Dword(count) * stride), pool);
    };
    if (flags & 8) {
        allocate(100, 24);
        if (flags & 136)
            allocate(104, 24);
    } else {
        if (flags & 1) {
            allocate(100, 12);
            if (flags & 17)
                allocate(104, 12);
        }
        if (flags & 4) {
            allocate(108, 8);
            if (flags & 36)
                allocate(112, 8);
        }
        if (flags & 2) {
            allocate(116, 4);
            if (flags & 66)
                allocate(120, 4);
        }
    }
    allocate(124, 2);
    using namespace graphics::immediate;
    begin_model_builder(array);
    array[88 / 4] = array[92 / 4] = array[96 / 4] = 0;
    array[84 / 4] = array[76 / 4];
    bind_builder_primitive(array, primitive);
    builder_topology(array, topology);
    select_builder_parameters(array, "Parameters");
    bind_builder_parameter(array, "Texture", texture, 48, -1);
    flags = array[80 / 4];
    if (flags & 8) {
        bind_builder_parameter(array, "InterleavedVertices", at<void *>(array, 100), 24, -1);
    } else {
        if (flags & 1)
            bind_builder_parameter(array, "Coordinates", at<void *>(array, 100), 12, -1);
        if (flags & 4)
            bind_builder_parameter(array, "UV", at<void *>(array, 108), 8, -1);
        if (flags & 2)
            bind_builder_parameter(array, "Colours", at<void *>(array, 116), 4, -1);
    }
    bind_builder_parameter(array, "Indices", at<void *>(array, 124), 2, -1);
    bind_builder_parameter(array, "Parameters", array + 84 / 4, 4, -1);
    bind_builder_parameter(array, "MyMatrix", graphics::backend::model_view_projection_matrix, 64,
                           -1);
    bind_builder_parameter(array, "ModelMatrix", graphics::backend::model_matrix, 64, -1);
    bind_builder_parameter(array, "ViewMatrix", graphics::backend::view_matrix, 64, -1);
    bind_builder_parameter(array, "ProjectionMatrix", graphics::backend::projection_matrix, 64,
                           -1);
    bind_builder_parameter(array, "FogParams", graphics::backend::fog_parameters, 16, -1);
    graphics::geoprim::primitive(render_state, topology);
    bind_builder_parameter(array, "State", render_state, 68, -1);
    model_builder_matrix(array, identity_matrix);
    stop_model_builder(array);
}
} // namespace nfs::game::runtime::skids::lifecycle::pending

namespace nfs::graphics::immediate {

void draw_array(Dword *array, int requested_count) {
    begin_model_builder(array);
    Dword &flags = array[80 / 4];
    if (flags & 8) {
        if (flags & 8388608) {
            flags = (flags ^ 524288) & ~Dword(8388608);
            bind_builder_parameter(array, "InterleavedVertices",
                reinterpret_cast<void *>(array[100 / 4 + ((flags >> 19) & 1)]), 24, -1);
        }
    } else {
        if (flags & 1048576) {
            flags = (flags ^ 65536) & ~Dword(1048576);
            bind_builder_parameter(array, "Coordinates",
                reinterpret_cast<void *>(array[100 / 4 + ((flags >> 16) & 1)]), 12, -1);
        }
        if (flags & 2097152) {
            flags = (flags ^ 131072) & ~Dword(2097152);
            bind_builder_parameter(array, "UV",
                reinterpret_cast<void *>(array[108 / 4 + ((flags >> 17) & 1)]), 8, -1);
        }
        if (flags & 4194304) {
            flags = (flags ^ 262144) & ~Dword(4194304);
            bind_builder_parameter(array, "Colours",
                reinterpret_cast<void *>(array[116 / 4 + ((flags >> 18) & 1)]), 4, -1);
        }
    }
    stop_model_builder(array);
    if (!(flags & 256)) {
        Dword count = requested_count == -1 ? array[76 / 4] : Dword(requested_count);
        if (count)
            render_model_builder(array, count);
        flags |= 16777216;
    }
}

void *draw_array_vertices(Dword *array) {
    Dword flags = array[80 / 4];
    if ((flags & 16777352) == 16777352) {
        flags |= 8388608;
        array[80 / 4] = flags;
        return reinterpret_cast<void *>(array[100 / 4 + ((~flags >> 19) & 1)]);
    }
    return reinterpret_cast<void *>(array[100 / 4]);
}
} // namespace nfs::graphics::immediate
