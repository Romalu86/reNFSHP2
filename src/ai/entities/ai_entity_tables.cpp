#include "ai_entity_runtime.hpp"
namespace nfs::game::runtime::ai::entities {

const Dword body_base_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                  reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                  reinterpret_cast<Dword>(body_heading),
                                  reinterpret_cast<Dword>(body_position)};

const Dword entity_cast_base_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(base_traffic_view)};

const Dword entity_kind_base_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call)};

const Dword path_body_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                  reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                  reinterpret_cast<Dword>(body_heading),
                                  reinterpret_cast<Dword>(path_position_entry)};

const Dword path_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(path_owner_view)};

const Dword path_kind_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call)};

const Dword path_entity_methods[] = {reinterpret_cast<Dword>(entity_position),
                                    reinterpret_cast<Dword>(path_current_section_entry),
                                    reinterpret_cast<Dword>(path_current_sample_entry),
                                    reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                    reinterpret_cast<Dword>(base_collision_event),
                                    reinterpret_cast<Dword>(delete_path_entity),
                                    reinterpret_cast<Dword>(set_path_entity_active),
                                    reinterpret_cast<Dword>(base_accept_event),
                                    reinterpret_cast<Dword>(player_entity_view),
                                    reinterpret_cast<Dword>(racer_entity_view),
                                    reinterpret_cast<Dword>(perpetrator_entity_view),
                                    reinterpret_cast<Dword>(cop_entity_view),
                                    reinterpret_cast<Dword>(reset_path_entity),
                                    reinterpret_cast<Dword>(base_is_active),
                                    reinterpret_cast<Dword>(base_direction_event),
                                    reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                    reinterpret_cast<Dword>(move_towards_position)};

const Dword entity_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(base_traffic_view)};

const Dword entity_kind_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call)};

const Dword entity_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(base_route_section),
                               reinterpret_cast<Dword>(base_route_sample),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(base_collision_event),
                               reinterpret_cast<Dword>(delete_entity),
                               reinterpret_cast<Dword>(set_entity_active),
                               reinterpret_cast<Dword>(base_accept_event),
                               reinterpret_cast<Dword>(player_entity_view),
                               reinterpret_cast<Dword>(racer_entity_view),
                               reinterpret_cast<Dword>(perpetrator_entity_view),
                               reinterpret_cast<Dword>(cop_entity_view),
                               reinterpret_cast<Dword>(reset_entity),
                               reinterpret_cast<Dword>(base_is_active)};

const Dword helicopter_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(helicopter_owner_entry)};

const Dword helicopter_kind_methods[] = {reinterpret_cast<Dword>(helicopter_kind_entry)};

const Dword helicopter_body_methods[] = {reinterpret_cast<Dword>(helicopter_body_entry),
                                        reinterpret_cast<Dword>(helicopter_mutable_body_entry),
                                        reinterpret_cast<Dword>(helicopter_heading_entry),
                                        reinterpret_cast<Dword>(helicopter_position_entry)};

const Dword helicopter_methods[] = {reinterpret_cast<Dword>(entity_position),
                                   reinterpret_cast<Dword>(path_current_section_entry),
                                   reinterpret_cast<Dword>(path_current_sample_entry),
                                   reinterpret_cast<Dword>(advance_helicopter),
                                   reinterpret_cast<Dword>(steer_helicopter),
                                   reinterpret_cast<Dword>(delete_helicopter),
                                   reinterpret_cast<Dword>(set_helicopter_active),
                                   reinterpret_cast<Dword>(base_accept_event),
                                   reinterpret_cast<Dword>(player_entity_view),
                                   reinterpret_cast<Dword>(racer_entity_view),
                                   reinterpret_cast<Dword>(perpetrator_entity_view),
                                   reinterpret_cast<Dword>(cop_entity_view),
                                   reinterpret_cast<Dword>(reset_helicopter),
                                   reinterpret_cast<Dword>(base_is_active),
                                   reinterpret_cast<Dword>(base_direction_event),
                                   reinterpret_cast<Dword>(helicopter_contact_mask),
                                   reinterpret_cast<Dword>(move_towards_position),
                                   reinterpret_cast<Dword>(helicopter_accelerator),
                                   reinterpret_cast<Dword>(helicopter_brake),
                                   reinterpret_cast<Dword>(helicopter_steering),
                                   reinterpret_cast<Dword>(helicopter_mutable_velocity),
                                   reinterpret_cast<Dword>(helicopter_velocity),
                                   reinterpret_cast<Dword>(helicopter_speed),
                                   reinterpret_cast<Dword>(set_entity_velocity),
                                   reinterpret_cast<Dword>(set_helicopter_velocity),
                                   reinterpret_cast<Dword>(helicopter_body_rotation),
                                   reinterpret_cast<Dword>(helicopter_rotation),
                                   reinterpret_cast<Dword>(choose_helicopter_route),
                                   reinterpret_cast<Dword>(helicopter_flight_state),
                                   reinterpret_cast<Dword>(entity_velocity),
                                   reinterpret_cast<Dword>(set_path_velocity),
                                   reinterpret_cast<Dword>(refresh_entity_route),
                                   reinterpret_cast<Dword>(brake_path_entity),
                                   reinterpret_cast<Dword>(set_helicopter_speed),
                                   reinterpret_cast<Dword>(helicopter_entity_view)};

const Dword flight_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(flight_owner_view)};

const Dword flight_kind_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call)};

const Dword flight_body_methods[] = {
    reinterpret_cast<Dword>(flight_body_view), reinterpret_cast<Dword>(flight_mutable_body_view),
    reinterpret_cast<Dword>(body_heading), reinterpret_cast<Dword>(flight_position)};

const Dword flight_methods[] = {reinterpret_cast<Dword>(entity_position),
                               reinterpret_cast<Dword>(path_current_section_entry),
                               reinterpret_cast<Dword>(path_current_sample_entry),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(base_collision_event),
                               reinterpret_cast<Dword>(delete_flight_entity),
                               reinterpret_cast<Dword>(set_path_entity_active),
                               reinterpret_cast<Dword>(base_accept_event),
                               reinterpret_cast<Dword>(player_entity_view),
                               reinterpret_cast<Dword>(racer_entity_view),
                               reinterpret_cast<Dword>(perpetrator_entity_view),
                               reinterpret_cast<Dword>(cop_entity_view),
                               reinterpret_cast<Dword>(reset_path_entity),
                               reinterpret_cast<Dword>(base_is_active),
                               reinterpret_cast<Dword>(base_direction_event),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(move_towards_position),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(entity_forward_speed),
                               reinterpret_cast<Dword>(set_entity_velocity),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call),
                               reinterpret_cast<Dword>(choose_path_branch),
                               reinterpret_cast<Dword>(entity_uses_path),
                               reinterpret_cast<Dword>(entity_velocity),
                               reinterpret_cast<Dword>(set_path_velocity),
                               reinterpret_cast<Dword>(refresh_entity_route),
                               reinterpret_cast<Dword>(brake_path_entity),
                               reinterpret_cast<Dword>(assertions::pure_virtual_call)};

const Dword flight_body_base_methods[] = {
    reinterpret_cast<Dword>(flight_body_view), reinterpret_cast<Dword>(flight_mutable_body_view),
    reinterpret_cast<Dword>(body_heading), reinterpret_cast<Dword>(body_position)};

const Dword projectile_body_methods[] = {reinterpret_cast<Dword>(projectile_body_entry),
                                        reinterpret_cast<Dword>(projectile_mutable_body_entry),
                                        reinterpret_cast<Dword>(body_heading),
                                        reinterpret_cast<Dword>(projectile_position_entry)};

const Dword projectile_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(base_traffic_view)};

const Dword projectile_kind_methods[] = {reinterpret_cast<Dword>(projectile_kind_entry)};

const Dword projectile_methods[] = {
    reinterpret_cast<Dword>(shape_position),          reinterpret_cast<Dword>(base_route_section),
    reinterpret_cast<Dword>(base_route_sample),       reinterpret_cast<Dword>(advance_projectile),
    reinterpret_cast<Dword>(collide_projectile),      reinterpret_cast<Dword>(delete_projectile),
    reinterpret_cast<Dword>(set_projectile_active),   reinterpret_cast<Dword>(base_accept_event),
    reinterpret_cast<Dword>(player_entity_view),      reinterpret_cast<Dword>(racer_entity_view),
    reinterpret_cast<Dword>(perpetrator_entity_view), reinterpret_cast<Dword>(cop_entity_view),
    reinterpret_cast<Dword>(reset_projectile),        reinterpret_cast<Dword>(base_is_active)};

const Dword shape_body_methods[] = {
    reinterpret_cast<Dword>(shape_body_view), reinterpret_cast<Dword>(shape_mutable_body_view),
    reinterpret_cast<Dword>(body_heading), reinterpret_cast<Dword>(shape_position_entry)};

const Dword shape_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(base_traffic_view)};

const Dword shape_kind_methods[] = {reinterpret_cast<Dword>(assertions::pure_virtual_call)};

const Dword shape_entity_methods[] = {reinterpret_cast<Dword>(shape_position),
                                     reinterpret_cast<Dword>(base_route_section),
                                     reinterpret_cast<Dword>(base_route_sample),
                                     reinterpret_cast<Dword>(assertions::pure_virtual_call),
                                     reinterpret_cast<Dword>(base_collision_event),
                                     reinterpret_cast<Dword>(delete_shape_entity),
                                     reinterpret_cast<Dword>(set_shape_entity_active),
                                     reinterpret_cast<Dword>(base_accept_event),
                                     reinterpret_cast<Dword>(player_entity_view),
                                     reinterpret_cast<Dword>(racer_entity_view),
                                     reinterpret_cast<Dword>(perpetrator_entity_view),
                                     reinterpret_cast<Dword>(cop_entity_view),
                                     reinterpret_cast<Dword>(reset_entity),
                                     reinterpret_cast<Dword>(base_is_active)};

const Dword spike_body_methods[] = {
    reinterpret_cast<Dword>(spike_body_entry), reinterpret_cast<Dword>(spike_mutable_body_entry),
    reinterpret_cast<Dword>(body_heading), reinterpret_cast<Dword>(spike_position_entry)};

const Dword spike_cast_methods[] = {
    reinterpret_cast<Dword>(base_racer_view), reinterpret_cast<Dword>(base_player_view),
    reinterpret_cast<Dword>(base_cop_view),   reinterpret_cast<Dword>(base_perpetrator_view),
    reinterpret_cast<Dword>(base_net_view),   reinterpret_cast<Dword>(base_traffic_view)};

const Dword spike_kind_methods[] = {reinterpret_cast<Dword>(spike_kind_entry)};

const Dword spike_methods[] = {reinterpret_cast<Dword>(shape_position),
                              reinterpret_cast<Dword>(spike_route_section),
                              reinterpret_cast<Dword>(spike_route_sample),
                              reinterpret_cast<Dword>(advance_spikes),
                              reinterpret_cast<Dword>(base_collision_event),
                              reinterpret_cast<Dword>(delete_spikes),
                              reinterpret_cast<Dword>(set_spike_active),
                              reinterpret_cast<Dword>(base_accept_event),
                              reinterpret_cast<Dword>(player_entity_view),
                              reinterpret_cast<Dword>(racer_entity_view),
                              reinterpret_cast<Dword>(perpetrator_entity_view),
                              reinterpret_cast<Dword>(cop_entity_view),
                              reinterpret_cast<Dword>(reset_entity),
                              reinterpret_cast<Dword>(base_is_active)};
const std::int32_t entity_offsets[] = {-4, 216, 220};
const std::int32_t path_entity_offsets[] = {-4, 316, 324, 332};
const std::int32_t helicopter_offsets[] = {-4, 1700, 1708, 1680};
const std::int32_t flight_offsets[] = {0, 1364, 1384, 1392};
const std::int32_t projectile_flight_offsets[] = {0, 60};
const std::int32_t projectile_offsets[] = {-4, 264, 268, 276};
const std::int32_t spike_flight_offsets[] = {0, 84};
const std::int32_t spike_offsets[] = {-4, 288, 292, 300};
} // namespace nfs::game::runtime::ai::entities
