#include "gouraud_array_runtime.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::graphics::gouraud {
using game::runtime::at;
using game::runtime::bytes;
namespace {
extern const Dword methods[1];
Dword *builder(Dword *owner) {
    return at<Dword *>(owner, 4);
}
} // namespace

Dword *construct(Dword *owner, Dword vertices, Dword pool, Dword topology) {
    owner[4] = topology;
    owner[6] = vertices;
    owner[7] = pool | 1024;
    owner[1] = owner[2] = owner[3] = owner[5] = 0;
    owner[8] = reinterpret_cast<Dword>("State");
    owner[9] = reinterpret_cast<Dword>("WVP");
    owner[10] = reinterpret_cast<Dword>("Parameters");
    owner[11] = owner[692] = owner[693] = 0;
    owner[0] = reinterpret_cast<Dword>(methods);
    kernel::memory_zero(owner + 12, 680 * 4);
    auto *model =
        static_cast<Dword *>(nfs::lifecycle::allocate(76, "GouraudArray::mModel", owner[7]));
    if (model) {
        try {
            immediate::model_builder_construct(model);
        } catch (...) {
            gui::application_release(model);
            throw;
        }
    }
    owner[1] = reinterpret_cast<Dword>(model);
    auto *primitive =
        static_cast<Dword *>(nfs::lifecycle::allocate(76, "GouraudArray::mPrim", owner[7]));
    if (primitive) {
        try {
            auto *method = static_cast<Dword *>(eagl::allocator(44, "EAGL::Rendermethod new"));
            primitive[0] = reinterpret_cast<Dword>(
                method ? game::runtime::skids::lifecycle::pending::construct_render_method(
                             method, render_definition)
                       : nullptr);
            const char *names[] = {"EAGL::ViewPort::gpModelViewProjectionMatrix",
                                   "EAGL::ViewPort::gpModelMatrix", "EAGL::ViewPort::gpViewMatrix",
                                   "EAGL::ViewPort::gpProjectionMatrix",
                                   "&EAGL::RenderMethodConstants::gFogParameters"};
            for (unsigned index = 0; index < 5; ++index) {
                std::uint8_t found;
                primitive[4 + index * 2] =
                    eagl::pool_lookup(&eagl::external_symbols, names[index], &found);
                primitive[3 + index * 2] = 1;
            }
        } catch (...) {
            gui::application_release(primitive);
            throw;
        }
    }
    owner[3] = reinterpret_cast<Dword>(primitive);
    auto *state =
        static_cast<Dword *>(nfs::lifecycle::allocate(68, "GouraudArray::mPrimState", owner[7]));
    if (state) {
        try {
            nfs::lifecycle::construction::render_state(state);
        } catch (...) {
            gui::application_release(state);
            throw;
        }
    }
    owner[2] = reinterpret_cast<Dword>(state);
    auto *parameters =
        static_cast<Dword *>(kernel::memory_allocate("GouraudArray::mParams", 16, owner[7]));
    model = builder(owner);
    const Dword vertex_count = owner[6];
    parameters[0] = vertex_count;
    parameters[1] = parameters[2] = parameters[3] = 0;
    owner[5] = reinterpret_cast<Dword>(parameters);
    immediate::model_builder_value(model, vertex_count);
    immediate::builder_topology(model, owner[4]);
    immediate::bind_builder_primitive(model, at<Dword *>(owner, 12));
    immediate::bind_builder_parameter(model, reinterpret_cast<const char *>(owner[9]), backend::model_view_projection_matrix, 64,
                                      -1);
    immediate::bind_builder_parameter(model, "ModelMatrix", backend::model_matrix, 64, -1);
    immediate::bind_builder_parameter(model, "ViewMatrix", backend::view_matrix, 64, -1);
    immediate::bind_builder_parameter(model, "ProjectionMatrix", backend::projection_matrix, 64,
                                      -1);
    immediate::bind_builder_parameter(model, "FogParams", backend::fog_parameters, 16, -1);
    immediate::bind_builder_parameter(model, reinterpret_cast<const char *>(owner[10]), parameters, 16, -1);
    immediate::select_builder_parameters(model, reinterpret_cast<const char *>(owner[10]));
    auto append = [&](const char *name, Dword stride, Dword count) {
        const Dword index = owner[11];
        if (int(index) >= 10)
            return Dword{0};
        void *memory = kernel::memory_allocate("GouraudArray_mVars", int(count * stride), owner[7]);
        at<void *>(owner, 304 + index * 272) = memory;
        std::strcpy(reinterpret_cast<char *>(bytes(owner) + 48 + owner[11] * 272), name);
        at<Dword>(owner, 308 + owner[11] * 272) = stride;
        at<Dword>(owner, 312 + owner[11] * 272) = count;
        return ++owner[11];
    };
    const Dword indices = append("Indices", 2, vertex_count);
    if (indices && int(owner[6]) > 0) {
        void *descriptor = bytes(owner) + 48 + (indices - 1) * 272;
        void *memory = at<void *>(descriptor, 256);
        for (int index = 0; index < int(owner[6]); ++index) {
            if (memory) {
                at<std::uint16_t>(memory, at<Dword>(descriptor, 260) * Dword(index)) =
                    std::uint16_t(index);
                at<std::uint8_t>(descriptor, 268) = 1;
            }
        }
    }
    owner[692] = append("InterleavedVertices", 16, owner[6]);
    return owner;
}

void destroy(Dword *owner) {
    owner[0] = reinterpret_cast<Dword>(methods);
    if (Dword *model = builder(owner)) {
        immediate::model_builder_destroy(model);
        eagl::deallocator(model, 76);
        owner[1] = 0;
    }
    if (Dword *primitive = at<Dword *>(owner, 12)) {
        if (Dword *method = at<Dword *>(primitive, 0)) {
            game::runtime::skids::lifecycle::pending::destroy_render_method(method);
            eagl::deallocator(method, 44);
        }
        eagl::deallocator(primitive, 76);
        owner[3] = 0;
    }
    if (Dword *state = at<Dword *>(owner, 8)) {
        geoprim::destroy(state);
        eagl::deallocator(state, 68);
        owner[2] = 0;
    }
    if (void *parameters = at<void *>(owner, 20)) {
        kernel::memory_free(parameters);
        owner[5] = 0;
    }
    for (int index = 0; index < int(owner[11]); ++index)
        if (void *memory = at<void *>(owner, 304 + Dword(index) * 272)) {
            kernel::memory_free(memory);
            at<Dword>(owner, 304 + Dword(index) * 272) = 0;
        }
    owner[11] = 0;
}

void *__fastcall delete_array(Dword *owner, void *, std::uint8_t flags) {
    destroy(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}
namespace {
const Dword methods[1] = {reinterpret_cast<Dword>(delete_array)};
}

void set_state(Dword *owner, const Dword *state) {
    Dword *target = at<Dword *>(owner, 8);
    Dword *model = builder(owner);
    if (target && model) {
        immediate::begin_model_builder(model);
        geoprim::assign(target, state);
        geoprim::primitive(target, owner[4]);
        immediate::bind_builder_parameter(model, reinterpret_cast<const char *>(owner[8]), target,
                                          68, 1);
        immediate::stop_model_builder(model);
    }
}

void set_matrix(Dword *owner, const Dword *matrix) {
    immediate::model_builder_matrix(builder(owner), matrix);
}

void set_vertex(Dword *owner, Dword index, const float *position, const Dword *colour) {
    const Dword packed = *colour;
    const Dword y = reinterpret_cast<const Dword *>(position)[1];
    const Dword z = reinterpret_cast<const Dword *>(position)[2];
    if (!owner[692])
        return;
    void *descriptor = bytes(owner) + (owner[692] - 1) * 272;
    if (void *memory = at<void *>(descriptor, 304)) {
        auto *vertex =
            reinterpret_cast<Dword *>(bytes(memory) + at<Dword>(descriptor, 308) * index);
        vertex[0] = *reinterpret_cast<const Dword *>(position);
        vertex[1] = y;
        vertex[2] = z;
        vertex[3] = packed;
        at<std::uint8_t>(descriptor, 316) = 1;
    }
}

void commit(Dword *owner) {
    Dword *model = builder(owner);
    immediate::begin_model_builder(model);
    for (int index = 0; index < int(owner[11]); ++index) {
        void *descriptor = bytes(owner) + 48 + Dword(index) * 272;
        if (at<std::uint8_t>(descriptor, 268)) {
            immediate::bind_builder_parameter(
                model, static_cast<const char *>(descriptor), at<void *>(descriptor, 256),
                at<int>(descriptor, 260), at<int>(descriptor, 264));
            at<std::uint8_t>(descriptor, 268) = 0;
        }
    }
    immediate::stop_model_builder(model);
}

void draw(Dword *owner) {
    Dword *model = builder(owner);
    if (!model)
        return;
    immediate::begin_model_builder(model);
    for (int index = 0; index < int(owner[11]); ++index) {
        void *descriptor = bytes(owner) + 48 + Dword(index) * 272;
        if (at<std::uint8_t>(descriptor, 268)) {
            immediate::bind_builder_parameter(
                builder(owner), static_cast<const char *>(descriptor), at<void *>(descriptor, 256),
                at<int>(descriptor, 260), at<int>(descriptor, 264));
            at<std::uint8_t>(descriptor, 268) = 0;
        }
    }
    model = builder(owner);
    immediate::stop_model_builder(model);
    immediate::render_model_builder(model, model[56 / 4]);
}
} // namespace nfs::graphics::gouraud
