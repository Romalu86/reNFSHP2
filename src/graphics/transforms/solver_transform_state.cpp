#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::matrices {
const float *matrix{},*translation{};
Dword matrix_changed{},translation_changed{};
Dword cached_flags=Dword(-1),input_stride=12,output_stride=32;
}
namespace nfs::game::runtime::matrices::pending {

// in transform_dispatch_tables.cpp; eight optimized bodies remain unresolved.
Dword transform_projection_matrix{}, transform_clip_matrix{}, clip_flags{}, clip_aux{};
Dword transformed_mode{};
void *transform_vertex_buffer{}, *vertex_end{};
void *saved_vertices{}, *saved_end{}, *saved_output{};
const void *saved_input{};
Transform selected_transform_handler{};
Postprocess postprocess{};
}
