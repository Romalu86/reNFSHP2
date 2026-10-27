#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::game::runtime {
namespace pending {
// RET; the other emitter methods remain separate dependencies.
void reset_particle_type() {
}

void update_particles(Dword *object, Dword flags) {
    game::runtime::pending::update_world_particles(at<Dword *>(object, 12), flags);
    if (Byte(flags))
        update_car_particle_pool(at<Dword *>(object, 4));
}
} // namespace pending
} // namespace nfs::game::runtime
namespace nfs::lifecycle::table_entries {

void __fastcall reset_particle_level(System *object, void *) {
    game::runtime::reset_world_particle_pool(game::runtime::at<kernel::Dword *>(object, 12));
    game::runtime::reset_car_particle_pool(game::runtime::at<kernel::Dword *>(object, 4));
    game::runtime::reset_particle_buffers();
}
} // namespace nfs::lifecycle::table_entries
