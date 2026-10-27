#include "../pursuit/police_chase_x87.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "../../ai/lifecycle/ai_chase_tuning.hpp"
namespace nfs::game::runtime::ai::cars {
void release_cop_reservation(void *, void *);
void update_cop_patrol_target(void *, Dword);
using kernel::numeric::Extended;

void *contact_entity(void *entity, int index) {
    return at<void *>(entity, 12 + Dword(index) * 4);
}

Byte detect_speeding_racers(void *entity) {
    return police_frame::detect_speeding_racers_retained(entity);
}

void update_cop_patrol_target(void *entity, Dword controls) {
    police_frame::update_cop_patrol_target_retained(entity, controls);
}

void __fastcall autonomous_cop_recover_body(void *entity, void *, Dword controls_word) {
    police_frame::autonomous_cop_recover_body_retained(entity, nullptr, controls_word);
}
} // namespace nfs::game::runtime::ai::cars
