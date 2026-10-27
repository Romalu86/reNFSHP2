#include "nfs/graphics/textures/shape_conversion.hpp"
namespace nfs::graphics::textures::shape {

char *shape_name(Dword *image) {
    while (image) {
        if (Byte(*image) == 'p')
            return reinterpret_cast<char *>(image + 1);
        int offset = static_cast<int>(*image) >> 8;
        if (!offset)
            break;
        image = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + offset);
    }
    return nullptr;
}

}
