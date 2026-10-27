#include "ai_car_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

Dword __fastcall car_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return car_const_owner(self, nullptr);
}

Dword __fastcall car_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return car_owner(self, nullptr);
}

Dword __fastcall car_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 244;
    return entities::path_owner(self);
}

Dword __fastcall car_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return car_const_body(self, nullptr);
}

Dword __fastcall car_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return car_mutable_body(self, nullptr);
}

Dword __fastcall car_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 580;
    return entities::entity_position(self, nullptr);
}

Dword __fastcall perpetrator_perpetrator_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return perpetrator_owner(self, nullptr);
}

Dword __fastcall autonomous_cop_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return autonomous_cop_type(self, nullptr);
}

Dword __fastcall autonomous_cop_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1296;
    return car_const_owner(self, nullptr);
}

Dword __fastcall autonomous_cop_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1296;
    return car_owner(self, nullptr);
}

Dword __fastcall autonomous_cop_perpetrator_cast_entry(void *self, void *) {
    self = bytes(self) + 60;
    return perpetrator_owner(self, nullptr);
}

Dword __fastcall autonomous_cop_network_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1292;
    return cop_base_owner(self, nullptr);
}

Dword __fastcall autonomous_cop_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1540;
    return entities::path_owner(self);
}

Dword __fastcall autonomous_cop_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1296;
    return car_const_body(self, nullptr);
}

Dword __fastcall autonomous_cop_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1296;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall autonomous_cop_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1876;
    return entities::entity_position(self, nullptr);
}

void __fastcall autonomous_cop_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1896;
    reset_autonomous_cop(self, nullptr);
}

void __fastcall autonomous_cop_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    autonomous_cop_bind_chase(self, nullptr, chase);
}

void __fastcall autonomous_cop_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 1292;
    cop_base_chase_outcome(self, nullptr, outcome);
}

void __fastcall autonomous_cop_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    autonomous_cop_clear_chase(self, nullptr, chase);
}

Dword __fastcall cop_base_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 4;
    return car_const_owner(self, nullptr);
}

Dword __fastcall cop_base_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 4;
    return car_owner(self, nullptr);
}

Dword __fastcall cop_base_network_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return cop_base_owner(self, nullptr);
}

Dword __fastcall cop_base_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 248;
    return entities::path_owner(self);
}

Dword __fastcall cop_base_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 4;
    return car_const_body(self, nullptr);
}

Dword __fastcall cop_base_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 4;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall cop_base_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 584;
    return entities::entity_position(self, nullptr);
}

void __fastcall cop_base_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 604;
    cop_base_reset(self, nullptr);
}

void __fastcall cop_base_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    cop_base_bind_chase(self, nullptr, chase);
}

void __fastcall cop_base_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    cop_base_chase_outcome(self, nullptr, outcome);
}

void __fastcall cop_base_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    cop_base_clear_chase(self, nullptr, chase);
}

Dword __fastcall player_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return player_type(self, nullptr);
}

Dword __fastcall player_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 84;
    return car_const_owner(self, nullptr);
}

Dword __fastcall player_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 84;
    return car_owner(self, nullptr);
}

Dword __fastcall player_cop_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 48;
    return pursuit_owner(self, nullptr);
}

Dword __fastcall player_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 328;
    return entities::path_owner(self);
}

Dword __fastcall player_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 84;
    return car_const_body(self, nullptr);
}

Dword __fastcall player_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 84;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall player_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 664;
    return entities::entity_position(self, nullptr);
}

void __fastcall player_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 684;
    reset_pursuit_car(self, nullptr);
}

void __fastcall player_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 48;
    pursuit_bind_chase(self, nullptr, chase);
}

void __fastcall player_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 48;
    pursuit_chase_outcome(self, nullptr, outcome);
}

void __fastcall player_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 48;
    pursuit_clear_chase(self, nullptr, chase);
}

Dword __fastcall racer_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 36;
    return car_const_owner(self, nullptr);
}

Dword __fastcall racer_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 36;
    return car_owner(self, nullptr);
}

Dword __fastcall racer_cop_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return pursuit_owner(self, nullptr);
}

Dword __fastcall racer_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 280;
    return entities::path_owner(self);
}

Dword __fastcall racer_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 36;
    return car_const_body(self, nullptr);
}

Dword __fastcall racer_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 36;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall racer_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 616;
    return entities::entity_position(self, nullptr);
}

void __fastcall racer_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 636;
    reset_racer(self, nullptr);
}

void __fastcall racer_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    pursuit_bind_chase(self, nullptr, chase);
}

void __fastcall racer_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    pursuit_chase_outcome(self, nullptr, outcome);
}

void __fastcall racer_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    pursuit_clear_chase(self, nullptr, chase);
}

Dword __fastcall pursuit_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return pursuit_type(self, nullptr);
}

Dword __fastcall pursuit_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 80;
    return car_const_owner(self, nullptr);
}

Dword __fastcall pursuit_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 80;
    return car_owner(self, nullptr);
}

Dword __fastcall pursuit_cop_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    return pursuit_owner(self, nullptr);
}

Dword __fastcall pursuit_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 324;
    return entities::path_owner(self);
}

Dword __fastcall pursuit_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 80;
    return car_const_body(self, nullptr);
}

Dword __fastcall pursuit_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 80;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall pursuit_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 660;
    return entities::entity_position(self, nullptr);
}

void __fastcall pursuit_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 680;
    reset_pursuit_car(self, nullptr);
}

void __fastcall pursuit_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    pursuit_bind_chase(self, nullptr, chase);
}

void __fastcall pursuit_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    pursuit_chase_outcome(self, nullptr, outcome);
}

void __fastcall pursuit_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    pursuit_clear_chase(self, nullptr, chase);
}

Dword __fastcall ai_cop_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return ai_cop_type(self, nullptr);
}

Dword __fastcall ai_cop_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 108;
    return car_const_owner(self, nullptr);
}

Dword __fastcall ai_cop_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 108;
    return car_owner(self, nullptr);
}

Dword __fastcall ai_cop_cop_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 72;
    return pursuit_owner(self, nullptr);
}

Dword __fastcall ai_cop_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 352;
    return entities::path_owner(self);
}

Dword __fastcall ai_cop_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 108;
    return car_const_body(self, nullptr);
}

Dword __fastcall ai_cop_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 108;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall ai_cop_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 688;
    return entities::entity_position(self, nullptr);
}

void __fastcall ai_cop_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 708;
    ai_cop_reset(self, nullptr);
}

void __fastcall ai_cop_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    ai_cop_bind_chase(self, nullptr, chase);
}

void __fastcall ai_cop_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 72;
    pursuit_chase_outcome(self, nullptr, outcome);
}

void __fastcall ai_cop_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    ai_cop_clear_chase(self, nullptr, chase);
}

Dword __fastcall network_car_route_option_entry(void *self, void *) {
    return autonomous_cop_route_option(self, nullptr);
}

Dword __fastcall network_car_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return network_car_type(self, nullptr);
}

Dword __fastcall network_car_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 32;
    return car_const_owner(self, nullptr);
}

Dword __fastcall network_car_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 32;
    return car_owner(self, nullptr);
}

Dword __fastcall network_car_network_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 28;
    return cop_base_owner(self, nullptr);
}

Dword __fastcall network_car_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 276;
    return entities::path_owner(self);
}

Dword __fastcall network_car_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 32;
    return car_const_body(self, nullptr);
}

Dword __fastcall network_car_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 32;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall network_car_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 612;
    return entities::entity_position(self, nullptr);
}

void __fastcall network_car_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 632;
    reset_network_car(self, nullptr);
}

void __fastcall network_car_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    network_car_bind_chase(self, nullptr, chase);
}

void __fastcall network_car_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 28;
    cop_base_chase_outcome(self, nullptr, outcome);
}

void __fastcall network_car_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    network_car_clear_chase(self, nullptr, chase);
}

void __fastcall racer_frame_entry(void *self, void *) {
    car_frame(self, nullptr);
}

Dword __fastcall ai_racer_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return ai_racer_type(self, nullptr);
}

Dword __fastcall ai_racer_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    return car_const_owner(self, nullptr);
}

Dword __fastcall ai_racer_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    return car_owner(self, nullptr);
}

Dword __fastcall ai_racer_cop_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 8;
    return pursuit_owner(self, nullptr);
}

Dword __fastcall ai_racer_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 288;
    return entities::path_owner(self);
}

Dword __fastcall ai_racer_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    return car_const_body(self, nullptr);
}

Dword __fastcall ai_racer_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 44;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall ai_racer_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 624;
    return entities::entity_position(self, nullptr);
}

void __fastcall ai_racer_reset_chase_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 644;
    ai_racer_reset(self, nullptr);
}

void __fastcall ai_racer_bind_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    ai_racer_bind_chase(self, nullptr, chase);
}

void __fastcall ai_racer_chase_outcome_entry(void *self, void *, Dword outcome) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 8;
    pursuit_chase_outcome(self, nullptr, outcome);
}

void __fastcall ai_racer_clear_chase_entry(void *self, void *, Dword chase) {
    self = entities::adjust_interface(self);
    ai_racer_clear_chase(self, nullptr, chase);
}

Dword __fastcall traffic_kind_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    return traffic_type(self, nullptr);
}

Dword __fastcall traffic_racer_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 28;
    return car_const_owner(self, nullptr);
}

Dword __fastcall traffic_player_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 28;
    return car_owner(self, nullptr);
}

Dword __fastcall traffic_traffic_cast_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 272;
    return entities::path_owner(self);
}

Dword __fastcall traffic_const_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 28;
    return car_const_body(self, nullptr);
}

Dword __fastcall traffic_mutable_body_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 28;
    return car_mutable_body(self, nullptr);
}

Dword __fastcall traffic_body_position_entry(void *self, void *) {
    self = entities::adjust_interface(self);
    self = bytes(self) - 608;
    return entities::entity_position(self, nullptr);
}
} // namespace nfs::game::runtime::ai::cars
