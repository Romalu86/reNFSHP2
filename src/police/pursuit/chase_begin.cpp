#include "police_chase_x87.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include <algorithm>
namespace nfs::game::runtime::ai::lifecycle::pending {
extern float chase_transition_seconds;

Dword chase_active(void *chase) {
    return at<Dword>(chase, 88) != 0;
}

void begin_police_chase(void *chase, void *cop, void *target, Dword level) {
    police_frame::begin_police_chase_retained(chase, cop, target, level);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
