#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::pending {
Dword *gob_definitions = nullptr;
Dword track_load_begin = 0, track_load_end = 0;
} // namespace nfs::game::world::pending
namespace nfs::game::world::dependency {
// All bytes in these specifically accessed BSS spans are zero at image load.
AnimationState animation_state{};
void *animation_factory = nullptr;
AnimationBlock animation_blocks[3]{};
std::uint64_t clock_origin = 0;
Dword *static_references = nullptr;
} // namespace nfs::game::world::dependency
