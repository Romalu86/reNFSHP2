#pragma once
#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::runtime::physics::edges {
extern Dword surface_filter; // Shared by the polygon edge collectors.
Byte contains_point(Dword *, const float *);
Dword crosses_directed_edge(const float *, const float *, const float *, const float *);
Byte conditional_edge_blocked(const float *, const float *, void *const *);
int collect_boundary_edge(void *, void *const *, float *const *, const float *, const float *);
Dword intersect_static_shape(const void *, const float *, const float *, float *, float *, Dword *);
Byte *static_cell(const float *);
} // namespace nfs::game::runtime::physics::edges
