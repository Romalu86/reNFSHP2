#pragma once
#include "../../network/vehicles/remote_car_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::game::runtime::physics::construction {
extern const Dword car_position_methods[], car_base_methods[];
void *construct_scalar_parameters(void *);
void *construct_response_parameters(void *);
void *construct_curve_parameters(void *);
void *construct_pose_parameters(void *);
void construct_control_state(void *);
void *fill_car_history(void *, const Dword *);
void initialize_wheel_state(void *, void *, void *, Dword, void *);
void clear_car_run_statistics(void *);
void *construct_car_run_statistics(void *);
void *construct_impact_state(void *, void *);
void __fastcall query_car_position(void *, void *, float *);
} // namespace nfs::game::runtime::physics::construction
