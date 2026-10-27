#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include <cstdlib>
namespace nfs::graphics::methods {

Dword *construct_definition(Dword *method, Dword *code, Dword *techniques, Dword dictionary,
                           const char *const *names, Dword flags) {
    method[2] = reinterpret_cast<Dword>(techniques);
    method[3] = dictionary;
    method[0] = reinterpret_cast<Dword>(code);
    method[5] = reinterpret_cast<Dword>(names);
    method[1] = 0;
    method[6] = flags;
    method[7] = method[8] = method[9] = 0;
    method[10] = Dword(-1);
    method[4] = reinterpret_cast<Dword>(resources::render_methods);
    resources::render_methods = method;
    return method;
}
} // namespace nfs::graphics::methods
namespace nfs::game::runtime::skids::lifecycle::pending {
Dword render_method_definition[11]{};
namespace {
Dword code[] = {
#include "render_shared_code.inc"
};
Dword techniques[] = {
#include "render_shared_techniques.inc"
};
const char *const names[] = {
#include "render_shared_names.inc"
};
} // namespace

void release_shared_render_definition() {
    destroy_render_method(render_method_definition);
}

void initialize_shared_render_definition() {
    graphics::methods::construct_definition(render_method_definition, code, techniques, 0, names,
                                            1);
    std::atexit(release_shared_render_definition);
}
} // namespace nfs::game::runtime::skids::lifecycle::pending
NFS_KERNEL_INITIALIZER(startup_initialize_shared_render_definition, ".CRT$XCU_NFS_00690300",
                       nfs::game::runtime::skids::lifecycle::pending::initialize_shared_render_definition);
