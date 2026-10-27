#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::lifecycle::table_entries {
void* __fastcall dedicated_console_delete_entry(System* object,void*,unsigned flags);
void* __fastcall net_crypt_delete_entry(System* object,void*,unsigned flags);
void* __fastcall fonts_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall vehicle_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall track_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall event_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall level_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall track_delete_entry(System* object,void*,unsigned flags);
void* __fastcall world_light_delete_entry(System* object,void*,unsigned flags);
void* __fastcall environment_delete_entry(System* object,void*,unsigned flags);
void* __fastcall ai_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall game_rules_delete_entry(System* object,void*,unsigned flags);
void* __fastcall car_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall audio_delete_entry(System* object,void*,unsigned flags);
void* __fastcall speech_delete_entry(System* object,void*,unsigned flags);
void* __fastcall audio_loader_delete_entry(System* object,void*,unsigned flags);
void* __fastcall particle_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall race_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall skeleton_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall crc_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall input_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall shadow_manager_delete_entry(System* object,void*,unsigned flags);
void* __fastcall platform_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall path_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall network_game_delete_entry(System* object,void*,unsigned flags);
void* __fastcall network_server_delete_entry(System* object,void*,unsigned flags);
void* __fastcall network_client_delete_entry(System* object,void*,unsigned flags);
void* __fastcall skid_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall camera_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall character_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall fog_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall frontend_delete_entry(System* object,void*,unsigned flags);
void* __fastcall replay_delete_entry(System* object,void*,unsigned flags);
void* __fastcall start_line_delete_entry(System* object,void*,unsigned flags);
void* __fastcall lod_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall feedback_system_delete_entry(System* object,void*,unsigned flags);
void* __fastcall system_destroy(System*,void*,unsigned);
}
