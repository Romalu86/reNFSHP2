#pragma once
#include "../../ai/driving/ai_driver_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::cars {
void draw_vehicle_marker(void *, void *, Dword, Dword, Dword);
void draw_cop_progress_marker(void *, void *);
} // namespace nfs::game::runtime::ai::cars
