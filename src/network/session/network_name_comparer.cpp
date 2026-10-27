#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/session/network_player_state.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::network::packets::pending {
Dword name_comparer{};
Dword command_name_comparer{};
std::uint16_t reconnect_port{}; // Proved two-byte BSS footprint.
namespace {
extern const Dword name_methods[];
}
// Thiscall comparator; both explicit arguments are stack-passed.
Dword __fastcall compare_name(Dword *, void *, const char *name, const ui::PlayerRecord *player) {
    return std::strcmp(name, player->name) == 0;
}

void destroy_name_comparer(Dword *object) {
    *object = reinterpret_cast<Dword>(
        lifecycle::construction::network::pending::comparer_interface_methods);
}

Dword *construct_name_comparer(Dword *object) {
    *object = reinterpret_cast<Dword>(
        lifecycle::construction::network::pending::comparer_interface_methods);
    *object = reinterpret_cast<Dword>(name_methods);
    return object;
}

void *__fastcall delete_name_comparer(Dword *object, void *, unsigned flags) {
    destroy_name_comparer(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}
namespace {

const Dword name_methods[] = {reinterpret_cast<Dword>(compare_name),
                             reinterpret_cast<Dword>(delete_name_comparer)};
} // namespace

void finalize_name_comparer() {
    destroy_name_comparer(&name_comparer);
}
// Registration uses CRT atexit, as at .
void initialize_name_comparer() {
    construct_name_comparer(&name_comparer);
    std::atexit(finalize_name_comparer);
}

void finalize_command_name_comparer() {
    destroy_name_comparer(&command_name_comparer);
}

void initialize_command_name_comparer() {
    construct_name_comparer(&command_name_comparer);
    std::atexit(finalize_command_name_comparer);
}
} // namespace nfs::network::packets::pending


NFS_KERNEL_INITIALIZER(startup_initialize_name_comparer, ".CRT$XCU_NFS_0069028C",
                       nfs::network::packets::pending::initialize_name_comparer);
NFS_KERNEL_INITIALIZER(startup_initialize_slot, ".CRT$XCU_NFS_00690290",
                       nfs::network::players::initialize_slot);
NFS_KERNEL_INITIALIZER(startup_initialize_command_name_comparer, ".CRT$XCU_NFS_00690268",
                       nfs::network::packets::pending::initialize_command_name_comparer);
