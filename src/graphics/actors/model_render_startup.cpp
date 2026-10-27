#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "model_render_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>

namespace nfs::graphics::model_render {

void construct_primitive_state() {
    lifecycle::construction::render_state(primitive_state);
}


void destroy_primitive_state() {
    geoprim::destroy(primitive_state);
}


void register_primitive_state_cleanup() {
    std::atexit(destroy_primitive_state);
}

 
void initialize_primitive_state() {
    construct_primitive_state();
    register_primitive_state_cleanup();
}
} // namespace nfs::graphics::model_render

NFS_KERNEL_INITIALIZER(startup_initialize_primitive_state, ".CRT$XCU_NFS_006904CC",
                       nfs::graphics::model_render::initialize_primitive_state);
