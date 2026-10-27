#pragma once
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::model_render {
using Byte = std::uint8_t;
template <class T> T &field(void *object, unsigned offset) {
    return *reinterpret_cast<T *>(static_cast<Byte *>(object) + offset);
}
template <class T> T *pointer(Dword value) {
    return reinterpret_cast<T *>(value);
}
extern Dword *parameters, *parameter_start, *commands;
extern Dword index_count, vertex_count, primitive_count, retained_buffers;
extern Dword primitive_state[17];
extern Dword *technique_owner, *technique_state;
extern Dword dynamic_usage, index_offset;
extern Byte identity_transforms;
extern float bounds_matrix[16], transposed_matrix[16], skin_matrices[129][16];
extern Dword *blend_weights;
extern int blend_count, three_weights, two_weights, one_weight;
using Command = void (*)();
extern const Command command_table[78];
void draw_method(void *, Dword *);
void set_bounds(const float *, const float *);
void patch_values(const Dword *);
void prepend_matrix(float *, const float *);
Byte sphere_visible(Dword *, const float *, float);
void transpose_matrix(float *, const float *);
void upload_skin_constants();
void upload_skin_constants_sse();
void upload_skin_constants_packed();
void transpose_skin_constants();
void transpose_skin_constants_sse();
void transpose_skin_constants_packed();
void set_skin_palette();
void upload_skin_constants_variant();
void upload_skin_constants_sse_variant();
void upload_skin_constants_packed_variant();
void transpose_skin_constants_variant();
void transpose_skin_constants_sse_variant();
void transpose_skin_constants_packed_variant();
extern Dword *active_target;
Dword *selected_target();
namespace pending {
float *__stdcall d3dx_multiply(float *, const float *, const float *);
void __stdcall d3dx_transpose(float *, const float *);
void configure_draw_pipeline();
void skin_vertex_stream();
void configure_lights();
} // namespace pending
} // namespace nfs::graphics::model_render
