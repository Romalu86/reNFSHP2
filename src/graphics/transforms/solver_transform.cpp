#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::matrices {

const float *set_matrix(const float *value) {
    const float *previous = matrix;
    matrix = value;
    if (value)
        matrix_changed = 1;
    else
        cached_flags = Dword(-1);
    return previous;
}

const float *set_translation(const float *value) {
    const float *previous = translation;
    translation = value;
    if (value)
        translation_changed = 1;
    else
        cached_flags = Dword(-1);
    return previous;
}

Dword set_output_stride(Dword value) {
    Dword previous = output_stride;
    output_stride = value;
    if (!value)
        cached_flags = Dword(-1);
    return previous;
}
// Preserve the dispatcher, cache and indirect providers.
Dword transform(Dword flags, int count, const void *input, void *output) {
    using namespace pending;
    if (cached_flags != flags) {
        if (!translation)
            flags &= ~4u;
        if (!matrix)
            flags &= ~2u;
        if (!transform_projection_matrix)
            flags &= ~65536u;
        if (!input_stride)
            input_stride = 12;
        if (!output_stride)
            output_stride = 32;
        if (!transform_clip_matrix || !clip_flags)
            flags &= ~131072u;
        if ((flags & 196608) == 196608 && !clip_aux)
            flags &= ~131072u;
        if ((flags & 1) && (flags & 131072))
            flags &= ~131072u;
        if (!transform_vertex_buffer)
            flags &= ~1u;
        transformed_mode = flags & 268435456u;
        selected_transform_handler = transforms[flags & 31];
        cached_flags = flags;
        postprocess = (flags & 196608)
                                   ? postprocessors[((flags >> 15) & 7) | (flags & 1)]
                                   : nullptr;
    }
    if (count <= 0 || !input || !output)
        return 0;
    if (selected_transform_handler) {
        saved_vertices = transform_vertex_buffer;
        saved_end = vertex_end;
        saved_input = input;
        saved_output = output;
        selected_transform_handler(count, input, output);
    }
    if (postprocess)
        postprocess(count, output);
    return flags;
}

void transform_vectors(int count, const float *input, const float *rotation, float *output) {
    const float *previous = set_matrix(rotation);
    Dword stride = set_output_stride(12);
    transform(2, count, input, output);
    set_matrix(previous);
    set_output_stride(stride);
}

void transform_positions(int count, const float *input, const float *rotation,
                         const float *position, float *output, Dword stride) {
    const float *previous_matrix = set_matrix(rotation),
                *previous_position = set_translation(position);
    Dword previous_stride = set_output_stride(stride);
    transform(6, count, input, output);
    set_matrix(previous_matrix);
    set_translation(previous_position);
    set_output_stride(previous_stride);
}
} // namespace nfs::game::runtime::matrices
