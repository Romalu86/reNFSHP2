#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::cars {

void __fastcall ai_cop_frame(void *entity, void *) {
    police_frame::ai_cop_frame_retained(entity, nullptr);
}
} // namespace nfs::game::runtime::ai::cars
