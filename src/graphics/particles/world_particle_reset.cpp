#include "nfs/world/resources/runtime_resources.hpp"
namespace nfs::game::runtime {
namespace pending {
void reset_particle_type();
}

void reset_world_particle_pool(Dword *object) {
    const int count = at<int>(object, 804);
    for (int index = 0; index < count; ++index)
        pending::reset_particle_emitter(at<Dword *>(object, 404 + index * 4));
    const int types = at<int>(object, 400);
    for (int index = 0; index < types; ++index)
        pending::reset_particle_type();
}
} // namespace nfs::game::runtime
