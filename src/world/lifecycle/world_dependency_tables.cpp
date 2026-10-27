#include "nfs/core/errors/assertions.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/network/session/network_virtual.hpp"
namespace nfs::game::world::dependency {
namespace adapters {
void *__fastcall delete_list(void *p, void *, Dword f) {
    return delete_actor_list(p, f);
}
void *__fastcall delete_list_base(void *p, void *, Dword f) {
    return delete_actor_list_base(p, f);
}
void *__fastcall delete_characters(void *p, void *, Dword f) {
    return delete_character_table(p, f);
}
void __fastcall append(void *p, void *, void *n) {
    append_actor_node(p, n);
}
void *__fastcall delete_line(void *p, void *, Dword f) {
    return delete_start_line(p, f);
}
void *__fastcall delete_modes(void *p, void *, Dword f) {
    return world::delete_camera_modes(p, f);
}
void *__fastcall delete_director(void *p, void *, Dword f) {
    return delete_camera_director(p, f);
}
void __fastcall update_director(void *p, void *) {
    pending::update_camera_director(p);
}
void *__fastcall current_state(void *p, void *) {
    return current_camera_state(p);
}
void __fastcall initialize_director(void *p, void *) {
    pending::initialize_camera_director(p);
}
void __fastcall unload_director(void *p, void *) {
    pending::unload_camera_director(p);
}
void *__fastcall delete_boom_base(void *p, void *, Dword f) {
    return delete_base_boom(p, f);
}
void __fastcall boom_position(void *p, void *, Dword *o) {
    read_boom_position(p, o);
}
void *__fastcall delete_booms(void *p, void *, Dword f) {
    return delete_boom_array(p, f);
}
void __fastcall boom_collision(void *p, void *, Dword a, void *b) {
    pending::draw_boom(p, a, b);
}
void __fastcall step_boom(void *p, void *) {
    pending::advance_boom(p);
}
void *__fastcall definition(void *p, void *) {
    return boom_definition(p);
}
void __fastcall bounds(void *p, void *, Dword *o) {
    read_boom_bounds(p, o);
}
} // namespace adapters
} // namespace nfs::game::world::dependency
namespace nfs::game::world::pending {
namespace a = nfs::game::world::dependency::adapters;
// Exact table spans terminate at the next table/constant; all entries retain their actual target.
 
const Dword actor_list_methods[2] = {reinterpret_cast<Dword>(a::delete_list),
                                             reinterpret_cast<Dword>(a::append)};
const Dword actor_list_base_methods[2] = {reinterpret_cast<Dword>(a::delete_list_base),
                                                  reinterpret_cast<Dword>(a::append)};
const Dword character_table_methods[2] = {reinterpret_cast<Dword>(a::delete_characters),
                                                  reinterpret_cast<Dword>(a::append)};
const Dword start_line_methods[1] = {reinterpret_cast<Dword>(a::delete_line)};
const Dword camera_modes_methods[5] = {
    reinterpret_cast<Dword>(a::delete_modes), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call)};
const Dword camera_director_methods[5] = {
    reinterpret_cast<Dword>(a::delete_director), reinterpret_cast<Dword>(a::update_director),
    reinterpret_cast<Dword>(a::current_state), reinterpret_cast<Dword>(a::initialize_director),
    reinterpret_cast<Dword>(a::unload_director)};
const Dword boom_base_methods[6] = {
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call), reinterpret_cast<Dword>(a::delete_boom_base),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call)};
const Dword boom_methods[6] = {
    reinterpret_cast<Dword>(a::boom_position),  reinterpret_cast<Dword>(a::delete_booms),
    reinterpret_cast<Dword>(a::boom_collision), reinterpret_cast<Dword>(a::step_boom),
    reinterpret_cast<Dword>(a::definition),     reinterpret_cast<Dword>(a::bounds)};
} // namespace nfs::game::world::pending
