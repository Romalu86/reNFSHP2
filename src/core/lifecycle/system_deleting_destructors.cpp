#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include "nfs/network/session/network_construction.hpp"
#include "nfs/core/lifecycle/system_delete_entries.hpp"
namespace nfs::lifecycle::table_entries {

void* __fastcall dedicated_console_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::dedicated_console(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall net_crypt_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::net_crypt(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall fonts_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::fonts_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall vehicle_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::vehicle_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall track_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::track_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall event_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::event_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall level_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::level_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall track_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::track(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall world_light_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::world_light(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall environment_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::environment(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall ai_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::ai_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall game_rules_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::game_rules(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall car_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::car_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall audio_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::audio(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall speech_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::speech(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall audio_loader_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::audio_loader(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall particle_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::particle_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall race_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::race_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall skeleton_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::skeleton_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall crc_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::crc_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall input_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::input_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall shadow_manager_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::shadow_manager(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall platform_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::platform_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall path_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::path_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall network_game_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::network_game(fields);construction::destruction::network_game_base(fields+24);if(flags&1)gui::application_release(object);return object;}

void* __fastcall network_server_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::network_server(fields);fields[99]=reinterpret_cast<Dword>(construction::network::pending::visitor_interface_methods);construction::destruction::network_server_base(fields+80);if(flags&1)gui::application_release(object);return object;}

void* __fastcall network_client_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::network_client(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall skid_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::skid_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall camera_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::camera_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall character_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::character_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall fog_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::fog_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall frontend_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::frontend(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall replay_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::replay(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall start_line_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::start_line(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall lod_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::lod_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall feedback_system_delete_entry(System* object,void*,unsigned flags){auto* fields=reinterpret_cast<Dword*>(object);construction::destruction::feedback_system(fields);if(flags&1)gui::application_release(object);return object;}

void* __fastcall system_destroy(System* object,void*,unsigned flags){object->methods=&base_system_methods;if(flags&1)gui::application_release(object);return object;}
}
