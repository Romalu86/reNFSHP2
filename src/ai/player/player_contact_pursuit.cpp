#include "../../police/pursuit/police_chase_x87.hpp"
#include "../driving/ai_car_behavior.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/precision/retained_ai.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

Byte target_on_neighboring_section(void *entity, void *target) {
    void *section = invoke<void *>(target, 1);
    if (!section)
        return 0;
    if (section != lifecycle::path_current_section(bytes(entity) + 236) &&
        section != lifecycle::path_current_section(bytes(entity) + 248) &&
        section != lifecycle::path_current_section(bytes(entity) + 224))
        return 0;
    return 1;
}
// Defined result consumed in AL.
Dword __fastcall player_pursuit_event(void *entity, void *, Dword controls_word) {
    return police_frame::player_pursuit_event_retained(entity, nullptr, controls_word);
}

void __fastcall player_path_brake(void *entity, void *, Dword controls_word) {
    police_frame::player_path_brake_retained(entity, nullptr, controls_word);
}
} // namespace nfs::game::runtime::ai::cars
