#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::lifecycle::construction::destruction {
using kernel::Dword;
void dedicated_console(Dword*);void net_crypt(Dword*);void fonts_manager(Dword*);void vehicle_manager(Dword*);void track_manager(Dword*);
void event_manager(Dword*);void level_manager(Dword*);void track(Dword*);void world_light(Dword*);void environment(Dword*);void ai_system(Dword*);
void game_rules(Dword*);void car_manager(Dword*);void audio(Dword*);void speech(Dword*);void audio_loader(Dword*);void particle_system(Dword*);
void race_manager(Dword*);void skeleton_system(Dword*);void crc_system(Dword*);void input_system(Dword*);void shadow_manager(Dword*);void platform_system(Dword*);
void path_system(Dword*);void network_game(Dword*);void network_server(Dword*);void network_client(Dword*);void skid_system(Dword*);void camera_system(Dword*);
void character_system(Dword*);void fog_system(Dword*);void frontend(Dword*);void replay(Dword*);void start_line(Dword*);void lod_system(Dword*);void feedback_system(Dword*);
void network_game_base(Dword*);void network_server_base(Dword*);void thread(Dword*);
void vehicle_cache(Dword*);void vehicle_list(Dword*);void track_state(Dword*);void environment_target(Dword*);void environment_view(Dword*);
void render_state(Dword*);void render_state_member(Dword*);void light_record(Dword*);
}
