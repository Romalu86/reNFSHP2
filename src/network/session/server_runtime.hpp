#pragma once
#include "../../race/session/race_session.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/core/timing/frame_clock.hpp"
namespace nfs::application::pending {
extern Dword server_password_enabled;
void rotate_server_race(void *);
int next_server_track(void *);
void set_server_direction(void *, Dword);
void set_server_car_class(void *, int);
void save_server_settings(void *);
void reset_race_round(void *);
void reset_network_clocks();
extern network::players::Visitor clear_grid_visitor;
void unload_network_race(void *);
void load_network_race(void *);
} // namespace nfs::application::pending
