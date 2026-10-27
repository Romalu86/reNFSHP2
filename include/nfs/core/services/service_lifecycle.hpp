#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/graphics/backend/eagl.hpp"
#include <initializer_list>

namespace nfs::services {
using kernel::Dword;
using Byte = std::uint8_t;
template <class T> T &field(void *object, unsigned offset) {
    return *reinterpret_cast<T *>(static_cast<Byte *>(object) + offset);
}
template <class T> const T &field(const void *object, unsigned offset) {
    return *reinterpret_cast<const T *>(static_cast<const Byte *>(object) + offset);
}
void initialize_console(void *);
void initialize_environment(void *);
void shutdown_environment(void *);
void create_environment_targets(void *);
void release_environment_targets(void *);
void initialize_game_rules(void *);
void initialize_input(void *);
void load_input_level(void *);
void unload_input_level(void *);
void load_rumble_configuration();
void read_surface_feedback(void *);
void reset_input_state(void *);
void initialize_input_bindings(void *);
void set_input_binding(void *, int, Dword);
void initialize_shadow(void *);
void load_shadow_level(void *);
void unload_shadow_level(void *);
char *register_render_parameter(const char *, void *);
void initialize_paths(void *);
Dword create_user_directory(void *);
void initialize_network(void *);
void shutdown_network(void *);
void finish_network_round(void *);
void initialize_server(void *);
void initialize_client(void *);
void initialize_frontend(void *);
void shutdown_frontend(void *);
void load_force_feedback(void *);
void select_feedback_device(void *);
bool feedback_selected(Dword);
void create_feedback_effects(void *);
void update_network(void *);
void submit_impact(const float *, int, int, float, float);
int find_catalog_car(const void *, const char *);
Dword special_race_mode(const void *);
void *construct_transport(void *);
void destroy_transport(void *);
void bind_transport_messages(void *, void *);
void *construct_network_messages(void *);
void destroy_network_messages(void *);
void set_server_race_setting(void *, Dword);
void set_gui_archive(void *);
void set_gui_repeat_timing(Dword, Dword);
void __cdecl pump_console_messages(Dword);
void destroy_console_queue(void *);
void destroy_console_window(void *);
void *__fastcall delete_console_queue(void *, void *, unsigned);
void *__fastcall delete_console_window(void *, void *, unsigned);
Dword pop_console_message(void *, void *);
LRESULT CALLBACK console_key_hook(int, WPARAM, LPARAM);

namespace pending {
// Exact launch/data addresses; owning storage and producers are shared with startup.
extern Dword &server_mode, &join_mode, &dedicated_server, &player_name_requested, &port_requested,
    &car_override, &opponents_override;
extern char (&player_name)[24], (&join_address)[24], (&port_text)[24], (&car_name)[24],
    (&opponents_text)[24];
extern Byte (&server_settings)[852];
extern Dword &network_kind;
extern void *&network_messages;
extern const Dword transport_methods[];
extern const Dword message_methods[];
extern Byte ambient_shadow[];
extern void *&gui_archive;
extern Dword &gui_repeat_interval, &gui_repeat_delay;
extern Byte input_event[424];
extern float *const rumble_values;
extern Byte *const surface_feedback;
extern Byte boom_sound_definitions[9628];
extern Byte (&impact_definition_data)[9364]; // Shared with boom sound loading.
extern const gui::Destroy console_queue_methods[], console_window_methods[];
extern HWND &console_window;
extern HHOOK &console_hook;
extern HINSTANCE &application_instance;
INT_PTR CALLBACK console_dialog(HWND, UINT, WPARAM, LPARAM);
void configure_input_devices(Dword, int);
void reset_input_device(void *, Dword);
void set_axis_binding(void *, Dword, Dword, Dword);
void set_button_binding(void *, Byte, Dword);
void prepare_network_transport(void *);
void stop_network_transport(void *);
void release_network_settings();
void load_network_settings();
void initialize_network_identity();
void shutdown_network_identity();
void initialize_network_filters();
void reset_network_filters();
void load_server_settings(void *);
void activate_server(void *, Dword);
void configure_client(void *, Dword);
void connect_client(void *, const char *, Dword, Dword, Dword);
void disconnect_failed_client(void *);
void initialize_frontend_services();
void load_frontend_screens(void *, Dword);
void create_reflection_target(void *, Dword, Dword, Dword, Dword, float, float, Dword);
void release_reflection_target(void *);
void initialize_reflection_camera(void *);
void destroy_reflection_mesh(void *);
void destroy_reflection_item(void *);
void transition_network(void *);
Byte car_is_remote(void *);
Byte client_player_left(void *, Dword);
void update_remote_car(void *, Dword, void *);
void tick_server(void *);
void tick_client(void *);
void tick_server_browser();
void tick_internet_services();
Byte enable_feedback(Dword);
void set_feedback_gain(Dword, Dword);
Dword create_feedback_effect(Dword, Dword *);
} // namespace pending
} // namespace nfs::services

namespace nfs::lifecycle::table_entries {
void __fastcall dedicated_console_initialize(System *, void *);
void __fastcall environment_initialize(System *, void *);
void __fastcall environment_shutdown(System *, void *);
void __fastcall game_rules_initialize(System *, void *);
void __fastcall input_initialize(System *, void *);
void __fastcall input_load_level(System *, void *);
void __fastcall input_unload_level(System *, void *);
void __fastcall shadow_initialize(System *, void *);
void __fastcall shadow_load_level(System *, void *);
void __fastcall shadow_unload_level(System *, void *);
void __fastcall paths_initialize(System *, void *);
void __fastcall network_initialize(System *, void *);
void __fastcall network_shutdown(System *, void *);
void __fastcall network_finish_round(System *, void *);
void __fastcall server_initialize(System *, void *);
void __fastcall client_initialize(System *, void *);
void __fastcall frontend_initialize(System *, void *);
void __fastcall frontend_shutdown(System *, void *);
void __fastcall force_feedback_load_level(System *, void *);
} // namespace nfs::lifecycle::table_entries
