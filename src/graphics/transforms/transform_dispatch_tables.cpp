#include "nfs/core/platform/cpu.hpp"
#include "nfs/graphics/transforms/transform_handlers.hpp"
namespace nfs::game::runtime::matrices::handlers {
namespace {
void dispatch_affine(int n, const void *i, void *o, bool indexed, bool project,
                     const float *position) {
    if (!matrices::pending::transformed_mode && Byte(kernel::cpu::amd_3d)) {
        prepare_packed();
        position = position == null_translation ? null_translation
                                                : reinterpret_cast<const float *>(prepared_vectors);
        if (indexed) {
            if (project)
                pending::packed_affine_project_indexed(n, i, o, position);
            else
                pending::packed_affine_indexed(n, i, o, position);
        } else if (project)
            pending::packed_affine_project(n, i, o, position);
        else
            pending::packed_affine(n, i, o, position);
    } else if (!matrices::pending::transformed_mode && Byte(kernel::cpu_sse)) {
        prepare_streaming();
        position = position == null_translation ? null_translation
                                                : reinterpret_cast<const float *>(prepared_vectors);
        if (indexed) {
            if (project)
                pending::streaming_affine_project_indexed(n, i, o, position);
            else
                pending::streaming_affine_indexed(n, i, o, position);
        } else if (project)
            pending::streaming_affine_project(n, i, o, position);
        else
            pending::streaming_affine(n, i, o, position);
    } else if (indexed) {
        if (project)
            scalar_affine_project_indexed(n, i, o, position);
        else
            scalar_affine_indexed(n, i, o, position);
    } else if (project)
        scalar_affine_project(n, i, o, position);
    else
        scalar_affine(n, i, o, position);
}
} // namespace

void rotate(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, false, false, null_translation);
}

void affine(int n, const void *i, void *o) { dispatch_affine(n, i, o, false, false, translation); }

void rotate_project(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, false, true, null_translation);
}

void affine_project(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, false, true, translation);
}

void rotate_indexed(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, true, false, null_translation);
}

void affine_indexed(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, true, false, translation);
}

void rotate_project_indexed(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, true, true, null_translation);
}

void affine_project_indexed(int n, const void *i, void *o) {
    dispatch_affine(n, i, o, true, true, translation);
}
} // namespace nfs::game::runtime::matrices::handlers
namespace nfs::game::runtime::matrices::pending {
using namespace handlers;
Transform transforms[32] = {nullptr,
                                     nullptr,
                                     rotate,
                                     rotate_indexed,
                                     translate,
                                     translate_indexed,
                                     affine,
                                     affine_indexed,
                                     project,
                                     project_indexed,
                                     rotate_project,
                                     rotate_project_indexed,
                                     translate_project,
                                     translate_project_indexed,
                                     affine_project,
                                     affine_project_indexed,
                                     nullptr,
                                     nullptr,
                                     rotate,
                                     rotate_indexed,
                                     translate,
                                     translate_indexed,
                                     affine,
                                     affine_indexed,
                                     capture_project,
                                     capture_project_indexed,
                                     capture_rotate_project,
                                     capture_rotate_project_indexed,
                                     capture_translate_project,
                                     capture_translate_project_indexed,
                                     capture_affine_project,
                                     capture_affine_project_indexed};
Postprocess postprocessors[8] = {nullptr, nullptr,         clip,   clip_indexed, repeat,
                                          nullptr, clip_and_repeat, nullptr};
} // namespace nfs::game::runtime::matrices::pending
