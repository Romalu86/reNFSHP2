#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::graphics::backend {
// Six consecutive 4x4 matrices.
Dword projection_matrix[16]{},model_matrix[16]{},view_projection_matrix[16]{},view_matrix[16]{},model_view_matrix[16]{},model_view_projection_matrix[16]{};
Dword screen_info[4]{};
TextureStageState texture_stage_state{},texture_stage_snapshot{};  
namespace unresolved {
Dword* secondary_methods=nullptr;Dword texture_cache[8]{},render_width=0,render_height=0;
std::uint8_t lighting_enabled=0,fog_enabled=0;Dword table_fog=0,vertex_fog=0,display_mode=1;
}
}
