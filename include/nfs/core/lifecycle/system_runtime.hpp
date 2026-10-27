#pragma once
#include "nfs/core/lifecycle/system_tables.hpp"
namespace nfs::lifecycle::runtime {
using kernel::Dword;
extern Dword *&particle_owner; // Shared with the constructor and emitter clients.
extern Dword &particle_emitters;  
extern void *animation_buffer;
extern volatile Dword animation_cursors[4];
extern Dword input_frame_counts[2];
extern Dword *console_objects[3];
extern CRITICAL_SECTION crypt_lock;
extern Dword *crypt_head;
void reset_world_lights(Dword *);
void build_crc_table(Dword *);
void reset_replay_record(void *);
} // namespace nfs::lifecycle::runtime
namespace nfs::lifecycle::table_entries {
void __fastcall initialize_lod(System *, void *);
void __fastcall initialize_particles(System *, void *);
void __fastcall shutdown_particles(System *, void *);
void __fastcall initialize_skids(System *, void *);
void __fastcall initialize_events(System *, void *);
void __fastcall initialize_speech(System *, void *);
void __fastcall initialize_skeleton(System *, void *);
void __fastcall enter_skeleton_frontend(System *, void *);
void __fastcall load_skeleton_level(System *, void *);
void __fastcall initialize_crc(System *, void *);
void __fastcall leave_camera_frontend(System *, void *);
void __fastcall leave_car_frontend(System *, void *);
void __fastcall load_network_level(System *, void *);
void __fastcall unload_network_level(System *, void *);
void __fastcall load_race_level(System *, void *);
void __fastcall reset_input_frame(System *, void *);
void __fastcall shutdown_track_manager(System *, void *);
void __fastcall track_update(System *, void *);
void __fastcall shutdown_client(System *, void *);
void __fastcall shutdown_console(System *, void *);
void __fastcall shutdown_netcrypt(System *, void *);
void __fastcall shutdown_feedback(System *, void *);
void __fastcall initialize_replay(System *, void *);
void __fastcall shutdown_replay(System *, void *);
void __fastcall load_replay_level(System *, void *);
void __fastcall unload_replay_level(System *, void *);
void __fastcall unload_start_line(System *, void *);
void __fastcall initialize_world_light(System *, void *);
void __fastcall enter_world_light_frontend(System *, void *);
void __fastcall leave_world_light_frontend(System *, void *);
void __fastcall initialize_fog(System *, void *);
void __fastcall load_fog_level(System *, void *);
} // namespace nfs::lifecycle::table_entries
