#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"

namespace nfs::services::pending {
Byte car_is_remote(void *car) {
    return game::runtime::physics::car_is_remote(car);
}
void destroy_reflection_mesh(void *mesh) {
    game::runtime::skids::lifecycle::destroy_array(static_cast<Dword *>(mesh));
}
} // namespace nfs::services::pending

namespace nfs::lifecycle::table_entries {
void __fastcall dedicated_console_initialize(System *p, void *) {
    services::initialize_console(p);
}
void __fastcall environment_initialize(System *p, void *) {
    services::initialize_environment(p);
}
void __fastcall environment_shutdown(System *p, void *) {
    services::shutdown_environment(p);
}
void __fastcall game_rules_initialize(System *p, void *) {
    services::initialize_game_rules(p);
}
void __fastcall input_initialize(System *p, void *) {
    services::initialize_input(p);
}
void __fastcall input_load_level(System *p, void *) {
    services::load_input_level(p);
}
void __fastcall input_unload_level(System *p, void *) {
    services::unload_input_level(p);
}
void __fastcall shadow_initialize(System *p, void *) {
    services::initialize_shadow(p);
}
void __fastcall shadow_load_level(System *p, void *) {
    services::load_shadow_level(p);
}
void __fastcall shadow_unload_level(System *p, void *) {
    services::unload_shadow_level(p);
}
void __fastcall paths_initialize(System *p, void *) {
    services::initialize_paths(p);
}
void __fastcall network_initialize(System *p, void *) {
    services::initialize_network(p);
}
void __fastcall network_shutdown(System *p, void *) {
    services::shutdown_network(p);
}
void __fastcall network_finish_round(System *p, void *) {
    services::finish_network_round(p);
}
void __fastcall server_initialize(System *p, void *) {
    services::initialize_server(p);
}
void __fastcall client_initialize(System *p, void *) {
    services::initialize_client(p);
}
void __fastcall frontend_initialize(System *p, void *) {
    services::initialize_frontend(p);
}
void __fastcall frontend_shutdown(System *p, void *) {
    services::shutdown_frontend(p);
}
void __fastcall force_feedback_load_level(System *p, void *) {
    services::load_force_feedback(p);
}
} // namespace nfs::lifecycle::table_entries
namespace nfs::game::runtime::pending {
void update_network_entry(Dword *object) {
    services::update_network(object);
}
} // namespace nfs::game::runtime::pending
