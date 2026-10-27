#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
namespace nfs::game::runtime::ai::lifecycle {
#include "ai_lifecycle_data.inc"
}
namespace nfs::game::runtime::ai::lifecycle::pending {
const std::int32_t net_profile_base_offsets[] = {0, 184};
float spawn_distance = 350.0f;
float helicopter_rotor_increment = 3.7999999523162841796875f;
Dword fractional_random_calls{};
Byte population_loading = 1;
Dword startup_ticks{};
Dword load_tick_4{}, load_tick_10{}, load_tick_end{};
} // namespace nfs::game::runtime::ai::lifecycle::pending
namespace nfs::game::runtime::ai::pending {
Dword *const path_controller = reinterpret_cast<Dword *>(lifecycle::storage + 748);
int &path_deadline = *reinterpret_cast<int *>(lifecycle::storage + 52);
} // namespace nfs::game::runtime::ai::pending
