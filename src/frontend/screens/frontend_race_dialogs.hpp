#pragma once
#include "frontend_menu_controls.hpp"
#include "frontend_dialogs.hpp"
#include "../../race/session/race_session.hpp"
#include "../../race/session/race_runtime.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {
extern const std::array<Method, 49> starting_grid_methods;
extern const std::array<Method, 47> letterbox_methods;
const char *__cdecl starting_grid_class_key();
const char *__cdecl letterbox_class_key();
void *create_starting_grid(void *);
void *create_letterbox(void *);
void draw_race_frame();
Dword __cdecl starting_grid_race_button(void *, Dword, Dword, Dword);
Dword profile_car_label(void *, Dword);
int __cdecl compare_grid_order(const void *, const void *);
}
