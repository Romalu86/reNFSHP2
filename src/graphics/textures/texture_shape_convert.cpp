#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
namespace nfs::graphics::textures {

Dword *convert_shape(Dword *image, Dword format) {
    Dword type = 120;
    for (const auto &row : formats)
        if (row.format == format) {
            type = row.shape;
            break;
        }
    return type == (*image & 255) ? image : shape::convert_type_default(image, type);
}
} // namespace nfs::graphics::textures
namespace nfs::graphics::textures::shape {
// This entry accepts a SHAPE type, not a D3D format.
Dword *convert_type_default(Dword *image, Dword type) {
    return convert_type(image, type, 1, -1, -1);
}
} // namespace nfs::graphics::textures::shape
