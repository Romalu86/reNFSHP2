#pragma once
#include "../entities/ai_entity_runtime.hpp"
namespace nfs::game::runtime::ai::entities::pending {
struct RouteCandidates {
    Dword sections[300];
    std::uint16_t count;
    std::uint16_t padding;
};
static_assert(sizeof(RouteCandidates) == 1204);
RouteCandidates *construct_route_candidates(RouteCandidates *);
void destroy_route_candidates(RouteCandidates *);
void append_route_vertex(void *, const float *);
void connect_route_vertices(void *, void *);
kernel::numeric::Extended find_vertex_route(void *, int, int, RouteCandidates *);
kernel::numeric::Extended trace_vertex_route(void *, int, RouteCandidates *);
void plan_graph_route(void *, void *, Dword, const float *, void *, int, const float *, void *,
                      int *, float *);
kernel::numeric::Extended horizontal_distance_squared(const float *, const float *);
kernel::numeric::Extended advance_path_distance(void *, float, int *, Dword *);
Dword sample_path_target(void *, float *, float *, Dword *, float *, float, const float *, Byte);
void reverse_entity_path(void *);
void select_helicopter_destination(void *);
Byte update_path_sample(void *, const float *);
Byte target_route_branch(void *, int *, float *, Dword *);
void release_chase_helicopter(void *);
void return_helicopter_to_patrol(void *);
void calculate_projectile_velocity(void *, float *);
void launch_projectile(void *, void *, const float *);
kernel::numeric::Extended direction_turns(float, float);
void play_projectile_impact(float, float, float, Dword, int);
kernel::numeric::Extended steer_toward_position(void *, const float *, void *);
} // namespace nfs::game::runtime::ai::entities::pending
