#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
#include "nfs/graphics/textures/shape_conversion.hpp"
namespace nfs::game::runtime::particles::pending {
// Negative signed indices retain the saved addressing behavior.
void fsh_directory_name(Dword *image, int index, Dword *output) {
    *output = index < static_cast<int>(image[2]) ? at<Dword>(image, 16 + Dword(index) * 8) : 0;
}

Dword *find_fsh_image(Dword *image, const char *name) {
    const int count = static_cast<int>(image[2]);
    for (int index = 0; index < count; ++index) {
        auto *block = reinterpret_cast<Dword *>(bytes(image) + at<Dword>(image, 20 + index * 8));
        const char *candidate = graphics::textures::shape::shape_name(block);
        char short_name[5];
        if (!candidate) {
            short_name[4] = 0;
            fsh_directory_name(image, index, reinterpret_cast<Dword *>(short_name));
            candidate = short_name;
        }
        if (!std::strcmp(candidate, name))
            return reinterpret_cast<Dword *>(bytes(image) + at<Dword>(image, 20 + index * 8));
    }
    return nullptr;
}
} // namespace nfs::game::runtime::particles::pending
