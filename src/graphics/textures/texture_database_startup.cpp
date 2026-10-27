#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include <cstdlib>

namespace nfs::graphics::textures {

void destroy_texture_database() {
    destroy(&database);
}

 
void initialize_texture_database() {
    construct(&database);
    std::atexit(destroy_texture_database);
}
} // namespace nfs::graphics::textures

NFS_KERNEL_INITIALIZER(startup_initialize_texture_database, ".CRT$XCU_NFS_006902A4",
                       nfs::graphics::textures::initialize_texture_database);
