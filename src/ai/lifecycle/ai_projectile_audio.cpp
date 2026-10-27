#include "../../police/pursuit/police_chase_x87.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "../entities/ai_entity_runtime.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstring>
namespace nfs::game::world::camera {
void state_orientation(void *, float *);
}
namespace nfs::game::runtime::ai::entities::pending {

void play_projectile_impact(float x, float y, float z, Dword event, int maximum_distance) {
    police_frame::play_projectile_impact_retained(x, y, z, event, maximum_distance);
}
} // namespace nfs::game::runtime::ai::entities::pending
