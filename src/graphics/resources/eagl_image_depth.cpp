#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::tar {
#include "eagl_image_tables.inc"

std::uint8_t image_depth(const Dword* image){return depth_table[*image&127];}
}
