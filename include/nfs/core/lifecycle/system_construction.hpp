#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
namespace nfs::lifecycle {
Manager* construct_manager(Manager*);
namespace construction {
using Fields=Dword;
Fields* net_crypt(Fields*);Fields* fonts_manager(Fields*);Fields* vehicle_manager(Fields*);Fields* track_manager(Fields*);
Fields* event_manager(Fields*);Fields* level_manager(Fields*);Fields* world_light(Fields*);Fields* environment(Fields*);
Fields* ai_system(Fields*);Fields* game_rules(Fields*);Fields* car_manager(Fields*);Fields* speech(Fields*);
Fields* particle_system(Fields*);Fields* skeleton_system(Fields*);Fields* crc_system(Fields*,Dword,Dword,Dword,Dword,std::uint8_t);
Fields* input_system(Fields*);Fields* shadow_manager(Fields*);Fields* platform_system(Fields*);
Fields* character_system(Fields*);Fields* fog_system(Fields*);Fields* replay_system(Fields*);Fields* start_line(Fields*);
Fields* lod_system(Fields*);Fields* feedback_system(Fields*);
Fields* render_state(Fields*);void identity_matrix(Fields*);void lod_settings(Fields*);
void construct_array(void*,int,int,void(*)(void*),void(*)(void*));
void construct_array(void*,int,int,void*(*)(void*),void(*)(void*));
Fields* network_game(Fields*,int);Fields* network_server(Fields*,int);Fields* network_client(Fields*);
namespace unresolved {
extern Dword graphics_screen_resolution_setting,graphics_colour_depth_setting,graphics_gamma_setting,reflection_and_driver_stats_shared_setting;
void character_callback();
extern gui::String shared_string;
}
}
}
