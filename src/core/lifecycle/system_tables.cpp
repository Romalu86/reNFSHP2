#include "nfs/core/errors/assertions.hpp"
#include "nfs/graphics/fonts/font_faces.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/network/packets/network_cipher.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include "nfs/core/lifecycle/system_delete_entries.hpp"
namespace nfs::lifecycle {
namespace table_entries {

void __fastcall dedicated_console_initialize(System *, void *);

void __fastcall shutdown_console(System *, void *);

void __fastcall dedicated_console_load_level_entry(System *, void *) {
}

void __fastcall dedicated_console_unload_level_entry(System *, void *) {
}

void __fastcall dedicated_console_reset_level_entry(System *, void *) {
}

void __fastcall shared_system_enter_frontend_entry(System *, void *) {
}

void __fastcall shared_system_leave_frontend_entry(System *, void *) {
}

void __fastcall shared_system_post_initialize_entry(System *, void *) {
}

void __fastcall initialize_netcrypt(System *object, void *) {
    nfs::network::cipher::initialize(object);
}

void __fastcall shutdown_netcrypt(System *, void *);

void __fastcall network_cipher_load_level_entry(System *, void *) {
}

void __fastcall network_cipher_unload_level_entry(System *, void *) {
}

void __fastcall network_cipher_reset_level_entry(System *, void *) {
}

void __fastcall initialize_fonts(System *, void *);

void __fastcall shutdown_fonts(System *, void *);

void __fastcall load_fonts_level(System *, void *);

void __fastcall unload_fonts_level(System *, void *);

void __fastcall fonts_reset_level_entry(System *, void *) {
}

void __fastcall enter_fonts_frontend(System *, void *);

void __fastcall leave_fonts_frontend(System *, void *);

void __fastcall vehicle_manager_initialize_entry(System *, void *) {
}

void __fastcall shutdown_vehicle_assets(System *, void *);

void __fastcall load_vehicle_level(System *, void *);

void __fastcall unload_vehicle_level(System *, void *);

void __fastcall reset_vehicle_level(System *, void *);

void __fastcall enter_vehicle_frontend(System *, void *);

void __fastcall leave_vehicle_frontend(System *, void *);

void __fastcall initialize_vehicle_catalog(System *, void *);

void __fastcall initialize_track_catalog(System *, void *);

void __fastcall shutdown_track_manager(System *, void *);

void __fastcall load_track_manager_level(System *, void *);

void __fastcall unload_track_manager_level(System *, void *);

void __fastcall track_update(System *, void *);

void __fastcall track_manager_enter_frontend_entry(System *, void *) {
}

void __fastcall track_manager_leave_frontend_entry(System *, void *) {
}

void __fastcall initialize_events(System *, void *);

void __fastcall events_shutdown_entry(System *, void *) {
}

void __fastcall load_event_lists(System *, void *);

void __fastcall events_unload_level_entry(System *, void *) {
}

void __fastcall clear_event_lists(System *, void *);

void __fastcall level_manager_initialize_entry(System *, void *) {
}

void __fastcall level_manager_shutdown_entry(System *, void *) {
}

void __fastcall load_level_resources(System *, void *);

void __fastcall unload_level_resources(System *, void *);

void __fastcall level_manager_reset_level_entry(System *, void *) {
}

void __fastcall level_manager_enter_frontend_entry(System *, void *) {
}

void __fastcall level_manager_leave_frontend_entry(System *, void *) {
}

void __fastcall track_initialize_entry(System *, void *) {
}

void __fastcall track_shutdown_entry(System *, void *) {
}

void __fastcall load_track_level(System *, void *);

void __fastcall unload_track_level(System *, void *);

void __fastcall reset_track_level(System *, void *);

void __fastcall track_enter_frontend_entry(System *, void *) {
}

void __fastcall track_leave_frontend_entry(System *, void *) {
}

void __fastcall initialize_world_light(System *, void *);

void __fastcall world_lights_shutdown_entry(System *, void *) {
}

void __fastcall load_world_lights(System *, void *);

void __fastcall world_lights_unload_level_entry(System *, void *) {
}

void __fastcall world_lights_reset_level_entry(System *, void *) {
}

void __fastcall enter_world_light_frontend(System *, void *);

void __fastcall leave_world_light_frontend(System *, void *);

void __fastcall environment_initialize(System *, void *);

void __fastcall environment_shutdown(System *, void *);

void __fastcall environment_reflections_load_level_entry(System *, void *) {
}

void __fastcall environment_reflections_unload_level_entry(System *, void *) {
}

void __fastcall environment_reflections_reset_level_entry(System *, void *) {
}

void __fastcall environment_reflections_enter_frontend_entry(System *, void *) {
}

void __fastcall environment_reflections_leave_frontend_entry(System *, void *) {
}

void __fastcall ai__initialize(System *, void *);

void __fastcall ai__shutdown(System *, void *);

void __fastcall ai__load_level(System *, void *);

void __fastcall ai__unload_level(System *, void *);

void __fastcall ai__reset_level(System *, void *);

void __fastcall game_rules_initialize(System *, void *);

void __fastcall game_rules_shutdown_entry(System *, void *) {
}

void __fastcall load_rule_network_state(System *, void *);

void __fastcall game_rules_unload_level_entry(System *, void *) {
}

void __fastcall game_rules_reset_level_entry(System *, void *) {
}

void __fastcall car_manager_initialize_entry(System *, void *) {
}

void __fastcall car_manager_shutdown_entry(System *, void *) {
}

void __fastcall load_car_level(System *, void *);

void __fastcall unload_car_level(System *, void *);

void __fastcall reset_car_level(System *, void *);

void __fastcall enter_car_frontend(System *, void *);

void __fastcall leave_car_frontend(System *, void *);

void __fastcall initialize_audio_client(System *, void *);

void __fastcall shutdown_audio_client(System *, void *);

void __fastcall load_audio_level(System *, void *);

void __fastcall unload_audio_level(System *, void *);

void __fastcall reset_audio_client(System *, void *);

void __fastcall enter_audio_frontend(System *, void *);

void __fastcall leave_audio_frontend(System *, void *);

void __fastcall initialize_speech(System *, void *);

void __fastcall shutdown_speech(System *, void *);

void __fastcall load_speech_level(System *, void *);

void __fastcall unload_speech_level(System *, void *);

void __fastcall reset_speech_race(System *, void *);

void __fastcall enter_speech_frontend(System *, void *);

void __fastcall leave_speech_frontend(System *, void *);

void __fastcall post_initialize_speech(System *, void *);

void __fastcall audio_loader_initialize_entry(System *, void *) {
}

void __fastcall audio_loader_shutdown_entry(System *, void *) {
}

void __fastcall load_audio_banks(System *, void *);

void __fastcall audio_loader_unload_level_entry(System *, void *) {
}

void __fastcall audio_loader_reset_level_entry(System *, void *) {
}

void __fastcall audio_loader_enter_frontend_entry(System *, void *) {
}

void __fastcall audio_loader_leave_frontend_entry(System *, void *) {
}

void __fastcall initialize_particles(System *, void *);

void __fastcall shutdown_particles(System *, void *);

void __fastcall particle_load_level(System *, void *);

void __fastcall particle_unload_level(System *, void *);

void __fastcall reset_particle_level(System *, void *);

void __fastcall race_manager_initialize_entry(System *, void *) {
}

void __fastcall race_manager_shutdown_entry(System *, void *) {
}

void __fastcall load_race_level(System *, void *);

void __fastcall unload_race_delegates(System *, void *);

void __fastcall reset_race_rules(System *, void *);

void __fastcall initialize_skeleton(System *, void *);

void __fastcall skeleton_system_shutdown_entry(System *, void *) {
}

void __fastcall load_skeleton_level(System *, void *);

void __fastcall skeleton_system_unload_level_entry(System *, void *) {
}

void __fastcall skeleton_system_reset_level_entry(System *, void *) {
}

void __fastcall enter_skeleton_frontend(System *, void *);

void __fastcall skeleton_system_leave_frontend_entry(System *, void *) {
}

void __fastcall initialize_crc(System *, void *);

void __fastcall crc_system_shutdown_entry(System *, void *) {
}

void __fastcall crc_system_load_level_entry(System *, void *) {
}

void __fastcall crc_system_unload_level_entry(System *, void *) {
}

void __fastcall crc_system_reset_level_entry(System *, void *) {
}

void __fastcall input_initialize(System *, void *);

void __fastcall input_system_shutdown_entry(System *, void *) {
}

void __fastcall input_load_level(System *, void *);

void __fastcall input_unload_level(System *, void *);

void __fastcall reset_input_frame(System *, void *);

void __fastcall shadow_initialize(System *, void *);

void __fastcall shadow_manager_shutdown_entry(System *, void *) {
}

void __fastcall shadow_load_level(System *, void *);

void __fastcall shadow_unload_level(System *, void *);

void __fastcall shadow_manager_reset_level_entry(System *, void *) {
}

void __fastcall platform_system_load_level_entry(System *, void *) {
}

void __fastcall platform_system_unload_level_entry(System *, void *) {
}

void __fastcall platform_system_reset_level_entry(System *, void *) {
}

void __fastcall paths_initialize(System *, void *);

void __fastcall path_system_shutdown_entry(System *, void *) {
}

void __fastcall path_system_load_level_entry(System *, void *) {
}

void __fastcall path_system_unload_level_entry(System *, void *) {
}

void __fastcall path_system_reset_level_entry(System *, void *) {
}

void __fastcall network_initialize(System *, void *);

void __fastcall network_shutdown(System *, void *);

void __fastcall load_network_level(System *, void *);

void __fastcall unload_network_level(System *, void *);

void __fastcall network_finish_round(System *, void *);

void __fastcall server_initialize(System *, void *);

void __fastcall shutdown_server_state(System *, void *);

void __fastcall network_server_load_level_entry(System *, void *) {
}

void __fastcall network_server_unload_level_entry(System *, void *) {
}

void __fastcall reset_server_round(System *, void *);

void __fastcall client_initialize(System *, void *);

void __fastcall shutdown_client(System *, void *);

void __fastcall network_client_load_level_entry(System *, void *) {
}

void __fastcall unload_client_state(System *, void *);

void __fastcall network_client_reset_level_entry(System *, void *) {
}

void __fastcall initialize_skids(System *, void *);

void __fastcall skid_shutdown(System *, void *) {
}

void __fastcall skid_load_level(System *, void *);

void __fastcall skid_unload_level(System *, void *);

void __fastcall reset_skid_pool(System *, void *);

void __fastcall cameras_initialize_entry(System *, void *) {
}

void __fastcall cameras_shutdown_entry(System *, void *) {
}

void __fastcall load_camera_level(System *, void *);

void __fastcall unload_camera_level(System *, void *);

void __fastcall reset_camera_level(System *, void *);

void __fastcall enter_camera_frontend(System *, void *);

void __fastcall leave_camera_frontend(System *, void *);

void __fastcall initialize_characters(System *, void *);

void __fastcall shutdown_characters(System *, void *);

void __fastcall load_character_level(System *, void *);

void __fastcall unload_character_level(System *, void *);

void __fastcall reset_character_level(System *, void *);

void __fastcall enter_character_frontend(System *, void *);

void __fastcall leave_character_frontend(System *, void *);

void __fastcall initialize_fog(System *, void *);

void __fastcall fog_shutdown_entry(System *, void *) {
}

void __fastcall load_fog_level(System *, void *);

void __fastcall fog_unload_level_entry(System *, void *) {
}

void __fastcall fog_reset_level_entry(System *, void *) {
}

void __fastcall frontend_initialize(System *, void *);

void __fastcall frontend_shutdown(System *, void *);

void __fastcall load_frontend_registry(System *, void *);

void __fastcall unload_frontend_registry(System *, void *);

void __fastcall frontend_reset_level_entry(System *, void *) {
}

void __fastcall initialize_replay(System *, void *);

void __fastcall shutdown_replay(System *, void *);

void __fastcall load_replay_level(System *, void *);

void __fastcall unload_replay_level(System *, void *);

void __fastcall reset_replay_mode(System *, void *);

void __fastcall start_line_initialize_entry(System *, void *) {
}

void __fastcall start_line_shutdown_entry(System *, void *) {
}

void __fastcall load_start_line(System *, void *);

void __fastcall unload_start_line(System *, void *);

void __fastcall start_line_reset_level_entry(System *, void *) {
}

void __fastcall initialize_lod(System *, void *);

void __fastcall level_of_detail_shutdown_entry(System *, void *) {
}

void __fastcall load_lod_level(System *, void *);

void __fastcall level_of_detail_unload_level_entry(System *, void *) {
}

void __fastcall level_of_detail_reset_level_entry(System *, void *) {
}

void __fastcall force_feedback_initialize_entry(System *, void *) {
}

void __fastcall shutdown_feedback(System *, void *);

void __fastcall force_feedback_load_level(System *, void *);

void __fastcall unload_feedback_effects(System *, void *);

void __fastcall reset_feedback_effects(System *, void *);

} // namespace table_entries
using namespace table_entries;

const SystemMethods dedicatedconsole_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::dedicated_console_delete_entry),
    reinterpret_cast<SystemCall>(dedicated_console_initialize),
    reinterpret_cast<SystemCall>(shutdown_console),
    reinterpret_cast<SystemCall>(dedicated_console_load_level_entry),
    reinterpret_cast<SystemCall>(dedicated_console_unload_level_entry),
    reinterpret_cast<SystemCall>(dedicated_console_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods netcryptsystem_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::net_crypt_delete_entry),
    reinterpret_cast<SystemCall>(initialize_netcrypt),
    reinterpret_cast<SystemCall>(shutdown_netcrypt),
    reinterpret_cast<SystemCall>(network_cipher_load_level_entry),
    reinterpret_cast<SystemCall>(network_cipher_unload_level_entry),
    reinterpret_cast<SystemCall>(network_cipher_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods fontsmanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::fonts_manager_delete_entry),
    reinterpret_cast<SystemCall>(initialize_fonts),
    reinterpret_cast<SystemCall>(shutdown_fonts),
    reinterpret_cast<SystemCall>(load_fonts_level),
    reinterpret_cast<SystemCall>(unload_fonts_level),
    reinterpret_cast<SystemCall>(fonts_reset_level_entry),
    reinterpret_cast<SystemCall>(enter_fonts_frontend),
    reinterpret_cast<SystemCall>(leave_fonts_frontend),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods vehiclemanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::vehicle_manager_delete_entry),
    reinterpret_cast<SystemCall>(vehicle_manager_initialize_entry),
    reinterpret_cast<SystemCall>(shutdown_vehicle_assets),
    reinterpret_cast<SystemCall>(load_vehicle_level),
    reinterpret_cast<SystemCall>(unload_vehicle_level),
    reinterpret_cast<SystemCall>(reset_vehicle_level),
    reinterpret_cast<SystemCall>(enter_vehicle_frontend),
    reinterpret_cast<SystemCall>(leave_vehicle_frontend),
    reinterpret_cast<SystemCall>(initialize_vehicle_catalog)};

const SystemMethods trackmanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::track_manager_delete_entry),
    reinterpret_cast<SystemCall>(initialize_track_catalog),
    reinterpret_cast<SystemCall>(shutdown_track_manager),
    reinterpret_cast<SystemCall>(load_track_manager_level),
    reinterpret_cast<SystemCall>(unload_track_manager_level),
    reinterpret_cast<SystemCall>(track_update),
    reinterpret_cast<SystemCall>(track_manager_enter_frontend_entry),
    reinterpret_cast<SystemCall>(track_manager_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods ceventmanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::event_manager_delete_entry),
    reinterpret_cast<SystemCall>(initialize_events),
    reinterpret_cast<SystemCall>(events_shutdown_entry),
    reinterpret_cast<SystemCall>(load_event_lists),
    reinterpret_cast<SystemCall>(events_unload_level_entry),
    reinterpret_cast<SystemCall>(clear_event_lists),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods levelmanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::level_manager_delete_entry),
    reinterpret_cast<SystemCall>(level_manager_initialize_entry),
    reinterpret_cast<SystemCall>(level_manager_shutdown_entry),
    reinterpret_cast<SystemCall>(load_level_resources),
    reinterpret_cast<SystemCall>(unload_level_resources),
    reinterpret_cast<SystemCall>(level_manager_reset_level_entry),
    reinterpret_cast<SystemCall>(level_manager_enter_frontend_entry),
    reinterpret_cast<SystemCall>(level_manager_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods track_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::track_delete_entry),
    reinterpret_cast<SystemCall>(track_initialize_entry),
    reinterpret_cast<SystemCall>(track_shutdown_entry),
    reinterpret_cast<SystemCall>(load_track_level),
    reinterpret_cast<SystemCall>(unload_track_level),
    reinterpret_cast<SystemCall>(reset_track_level),
    reinterpret_cast<SystemCall>(track_enter_frontend_entry),
    reinterpret_cast<SystemCall>(track_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods worldlightmanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::world_light_delete_entry),
    reinterpret_cast<SystemCall>(initialize_world_light),
    reinterpret_cast<SystemCall>(world_lights_shutdown_entry),
    reinterpret_cast<SystemCall>(load_world_lights),
    reinterpret_cast<SystemCall>(world_lights_unload_level_entry),
    reinterpret_cast<SystemCall>(world_lights_reset_level_entry),
    reinterpret_cast<SystemCall>(enter_world_light_frontend),
    reinterpret_cast<SystemCall>(leave_world_light_frontend),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods enviroreflection_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::environment_delete_entry),
    reinterpret_cast<SystemCall>(environment_initialize),
    reinterpret_cast<SystemCall>(environment_shutdown),
    reinterpret_cast<SystemCall>(environment_reflections_load_level_entry),
    reinterpret_cast<SystemCall>(environment_reflections_unload_level_entry),
    reinterpret_cast<SystemCall>(environment_reflections_reset_level_entry),
    reinterpret_cast<SystemCall>(environment_reflections_enter_frontend_entry),
    reinterpret_cast<SystemCall>(environment_reflections_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods ai_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::ai_system_delete_entry),
    reinterpret_cast<SystemCall>(ai__initialize),
    reinterpret_cast<SystemCall>(ai__shutdown),
    reinterpret_cast<SystemCall>(ai__load_level),
    reinterpret_cast<SystemCall>(ai__unload_level),
    reinterpret_cast<SystemCall>(ai__reset_level),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods gamerules_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::game_rules_delete_entry),
    reinterpret_cast<SystemCall>(game_rules_initialize),
    reinterpret_cast<SystemCall>(game_rules_shutdown_entry),
    reinterpret_cast<SystemCall>(load_rule_network_state),
    reinterpret_cast<SystemCall>(game_rules_unload_level_entry),
    reinterpret_cast<SystemCall>(game_rules_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods carmanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::car_manager_delete_entry),
    reinterpret_cast<SystemCall>(car_manager_initialize_entry),
    reinterpret_cast<SystemCall>(car_manager_shutdown_entry),
    reinterpret_cast<SystemCall>(load_car_level),
    reinterpret_cast<SystemCall>(unload_car_level),
    reinterpret_cast<SystemCall>(reset_car_level),
    reinterpret_cast<SystemCall>(enter_car_frontend),
    reinterpret_cast<SystemCall>(leave_car_frontend),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods audio_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::audio_delete_entry),
    reinterpret_cast<SystemCall>(initialize_audio_client),
    reinterpret_cast<SystemCall>(shutdown_audio_client),
    reinterpret_cast<SystemCall>(load_audio_level),
    reinterpret_cast<SystemCall>(unload_audio_level),
    reinterpret_cast<SystemCall>(reset_audio_client),
    reinterpret_cast<SystemCall>(enter_audio_frontend),
    reinterpret_cast<SystemCall>(leave_audio_frontend),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods speech_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::speech_delete_entry),
    reinterpret_cast<SystemCall>(initialize_speech),
    reinterpret_cast<SystemCall>(shutdown_speech),
    reinterpret_cast<SystemCall>(load_speech_level),
    reinterpret_cast<SystemCall>(unload_speech_level),
    reinterpret_cast<SystemCall>(reset_speech_race),
    reinterpret_cast<SystemCall>(enter_speech_frontend),
    reinterpret_cast<SystemCall>(leave_speech_frontend),
    reinterpret_cast<SystemCall>(post_initialize_speech)};

const SystemMethods audioloader_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::audio_loader_delete_entry),
    reinterpret_cast<SystemCall>(audio_loader_initialize_entry),
    reinterpret_cast<SystemCall>(audio_loader_shutdown_entry),
    reinterpret_cast<SystemCall>(load_audio_banks),
    reinterpret_cast<SystemCall>(audio_loader_unload_level_entry),
    reinterpret_cast<SystemCall>(audio_loader_reset_level_entry),
    reinterpret_cast<SystemCall>(audio_loader_enter_frontend_entry),
    reinterpret_cast<SystemCall>(audio_loader_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods particlesystem_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::particle_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_particles),
    reinterpret_cast<SystemCall>(shutdown_particles),
    reinterpret_cast<SystemCall>(particle_load_level),
    reinterpret_cast<SystemCall>(particle_unload_level),
    reinterpret_cast<SystemCall>(reset_particle_level),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods racemanager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::race_manager_delete_entry),
    reinterpret_cast<SystemCall>(race_manager_initialize_entry),
    reinterpret_cast<SystemCall>(race_manager_shutdown_entry),
    reinterpret_cast<SystemCall>(load_race_level),
    reinterpret_cast<SystemCall>(unload_race_delegates),
    reinterpret_cast<SystemCall>(reset_race_rules),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods skeleton_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::skeleton_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_skeleton),
    reinterpret_cast<SystemCall>(skeleton_system_shutdown_entry),
    reinterpret_cast<SystemCall>(load_skeleton_level),
    reinterpret_cast<SystemCall>(skeleton_system_unload_level_entry),
    reinterpret_cast<SystemCall>(skeleton_system_reset_level_entry),
    reinterpret_cast<SystemCall>(enter_skeleton_frontend),
    reinterpret_cast<SystemCall>(skeleton_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods crc_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::crc_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_crc),
    reinterpret_cast<SystemCall>(crc_system_shutdown_entry),
    reinterpret_cast<SystemCall>(crc_system_load_level_entry),
    reinterpret_cast<SystemCall>(crc_system_unload_level_entry),
    reinterpret_cast<SystemCall>(crc_system_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods input_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::input_system_delete_entry),
    reinterpret_cast<SystemCall>(input_initialize),
    reinterpret_cast<SystemCall>(input_system_shutdown_entry),
    reinterpret_cast<SystemCall>(input_load_level),
    reinterpret_cast<SystemCall>(input_unload_level),
    reinterpret_cast<SystemCall>(reset_input_frame),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods shadow_manager_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::shadow_manager_delete_entry),
    reinterpret_cast<SystemCall>(shadow_initialize),
    reinterpret_cast<SystemCall>(shadow_manager_shutdown_entry),
    reinterpret_cast<SystemCall>(shadow_load_level),
    reinterpret_cast<SystemCall>(shadow_unload_level),
    reinterpret_cast<SystemCall>(shadow_manager_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods platform_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::platform_system_delete_entry),
    reinterpret_cast<SystemCall>(platform_initialize),
    reinterpret_cast<SystemCall>(platform_shutdown),
    reinterpret_cast<SystemCall>(platform_system_load_level_entry),
    reinterpret_cast<SystemCall>(platform_system_unload_level_entry),
    reinterpret_cast<SystemCall>(platform_system_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods path_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::path_system_delete_entry),
    reinterpret_cast<SystemCall>(paths_initialize),
    reinterpret_cast<SystemCall>(path_system_shutdown_entry),
    reinterpret_cast<SystemCall>(path_system_load_level_entry),
    reinterpret_cast<SystemCall>(path_system_unload_level_entry),
    reinterpret_cast<SystemCall>(path_system_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods network_game_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::network_game_delete_entry),
    reinterpret_cast<SystemCall>(network_initialize),
    reinterpret_cast<SystemCall>(network_shutdown),
    reinterpret_cast<SystemCall>(load_network_level),
    reinterpret_cast<SystemCall>(unload_network_level),
    reinterpret_cast<SystemCall>(network_finish_round),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods server_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::network_server_delete_entry),
    reinterpret_cast<SystemCall>(server_initialize),
    reinterpret_cast<SystemCall>(shutdown_server_state),
    reinterpret_cast<SystemCall>(network_server_load_level_entry),
    reinterpret_cast<SystemCall>(network_server_unload_level_entry),
    reinterpret_cast<SystemCall>(reset_server_round),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods client_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::network_client_delete_entry),
    reinterpret_cast<SystemCall>(client_initialize),
    reinterpret_cast<SystemCall>(shutdown_client),
    reinterpret_cast<SystemCall>(network_client_load_level_entry),
    reinterpret_cast<SystemCall>(unload_client_state),
    reinterpret_cast<SystemCall>(network_client_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods skidsystem_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::skid_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_skids),
    reinterpret_cast<SystemCall>(skid_shutdown),
    reinterpret_cast<SystemCall>(skid_load_level),
    reinterpret_cast<SystemCall>(skid_unload_level),
    reinterpret_cast<SystemCall>(reset_skid_pool),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods camerasystem_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::camera_system_delete_entry),
    reinterpret_cast<SystemCall>(cameras_initialize_entry),
    reinterpret_cast<SystemCall>(cameras_shutdown_entry),
    reinterpret_cast<SystemCall>(load_camera_level),
    reinterpret_cast<SystemCall>(unload_camera_level),
    reinterpret_cast<SystemCall>(reset_camera_level),
    reinterpret_cast<SystemCall>(enter_camera_frontend),
    reinterpret_cast<SystemCall>(leave_camera_frontend),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods character_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::character_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_characters),
    reinterpret_cast<SystemCall>(shutdown_characters),
    reinterpret_cast<SystemCall>(load_character_level),
    reinterpret_cast<SystemCall>(unload_character_level),
    reinterpret_cast<SystemCall>(reset_character_level),
    reinterpret_cast<SystemCall>(enter_character_frontend),
    reinterpret_cast<SystemCall>(leave_character_frontend),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods fogsystem_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::fog_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_fog),
    reinterpret_cast<SystemCall>(fog_shutdown_entry),
    reinterpret_cast<SystemCall>(load_fog_level),
    reinterpret_cast<SystemCall>(fog_unload_level_entry),
    reinterpret_cast<SystemCall>(fog_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods frontend_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::frontend_delete_entry),
    reinterpret_cast<SystemCall>(frontend_initialize),
    reinterpret_cast<SystemCall>(frontend_shutdown),
    reinterpret_cast<SystemCall>(load_frontend_registry),
    reinterpret_cast<SystemCall>(unload_frontend_registry),
    reinterpret_cast<SystemCall>(frontend_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods replay_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::replay_delete_entry),
    reinterpret_cast<SystemCall>(initialize_replay),
    reinterpret_cast<SystemCall>(shutdown_replay),
    reinterpret_cast<SystemCall>(load_replay_level),
    reinterpret_cast<SystemCall>(unload_replay_level),
    reinterpret_cast<SystemCall>(reset_replay_mode),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods startline_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::start_line_delete_entry),
    reinterpret_cast<SystemCall>(start_line_initialize_entry),
    reinterpret_cast<SystemCall>(start_line_shutdown_entry),
    reinterpret_cast<SystemCall>(load_start_line),
    reinterpret_cast<SystemCall>(unload_start_line),
    reinterpret_cast<SystemCall>(start_line_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods lod_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::lod_system_delete_entry),
    reinterpret_cast<SystemCall>(initialize_lod),
    reinterpret_cast<SystemCall>(level_of_detail_shutdown_entry),
    reinterpret_cast<SystemCall>(load_lod_level),
    reinterpret_cast<SystemCall>(level_of_detail_unload_level_entry),
    reinterpret_cast<SystemCall>(level_of_detail_reset_level_entry),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods ffb_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::feedback_system_delete_entry),
    reinterpret_cast<SystemCall>(force_feedback_initialize_entry),
    reinterpret_cast<SystemCall>(shutdown_feedback),
    reinterpret_cast<SystemCall>(force_feedback_load_level),
    reinterpret_cast<SystemCall>(unload_feedback_effects),
    reinterpret_cast<SystemCall>(reset_feedback_effects),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};

const SystemMethods base_system_methods = {
    reinterpret_cast<decltype(SystemMethods::destroy)>(table_entries::system_destroy),
    reinterpret_cast<SystemCall>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<SystemCall>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<SystemCall>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<SystemCall>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<SystemCall>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<SystemCall>(shared_system_enter_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_leave_frontend_entry),
    reinterpret_cast<SystemCall>(shared_system_post_initialize_entry)};
} // namespace nfs::lifecycle

