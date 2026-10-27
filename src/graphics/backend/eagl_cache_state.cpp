#include "nfs/graphics/resources/eagl_cleanup.hpp"
namespace nfs::graphics::resources {
CacheState cache{};  
int vertex_capacity=255,index_capacity=255;  
Dword bound_streams[16]{},bound_index=0; // Sixteen cache entries and their active count.
Dword* render_methods=nullptr;  
Dword vertex_program_mode=0,pixel_program_mode=0;  
}
