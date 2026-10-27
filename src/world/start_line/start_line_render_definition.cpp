#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../graphics/primitives/gouraud_array_runtime.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include <cstdlib>
namespace nfs::graphics::gouraud {
Dword render_definition[11]{};
namespace {
Dword code[] = {
#include "start_line_render_code.inc"
};
Dword techniques[] = {
#include "start_line_render_techniques.inc"
};
const char *const names[] = {
#include "start_line_render_names.inc"
};
} // namespace

void release_render_definition() {
    game::runtime::skids::lifecycle::pending::destroy_render_method(render_definition);
}
 
void initialize_render_definition() {
    methods::construct_definition(render_definition, code, techniques, 0, names, 1);
    std::atexit(release_render_definition);
}
} // namespace nfs::graphics::gouraud
NFS_KERNEL_INITIALIZER(startup_initialize_render_definition, ".CRT$XCU_NFS_006902EC",
                       nfs::graphics::gouraud::initialize_render_definition);
