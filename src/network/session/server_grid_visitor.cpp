#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "server_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::pending {
network::players::Visitor clear_grid_visitor{};

Dword __fastcall clear_server_player_grid(network::players::Visitor *, void *,
                                         network::ui::PlayerRecord *player, Dword, Dword, Dword) {
    if (player->slot >= 0) {
        Dword empty[21]{};
        if (unsigned(player->slot) < 24)
            std::memcpy(grid_entry(system(72), Dword(player->slot)), empty, sizeof(empty));
        std::memcpy(&player->car, empty, sizeof(empty));
        player->slot = -1;
        at<Byte>(player, 120) = 0;
    }
    player->phase = 7;
    return 1;
}

void destroy_clear_grid_visitor(network::players::Visitor *visitor) {
    visitor->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

void *__fastcall delete_clear_grid_visitor(network::players::Visitor *visitor, void *, Dword flags) {
    destroy_clear_grid_visitor(visitor);
    if (flags & 1)
        gui::application_release(visitor);
    return visitor;
}

const Dword clear_grid_methods[] = {reinterpret_cast<Dword>(clear_server_player_grid),
                                   reinterpret_cast<Dword>(delete_clear_grid_visitor)};

network::players::Visitor *construct_clear_grid_visitor(network::players::Visitor *visitor) {
    visitor->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    visitor->methods = clear_grid_methods;
    return visitor;
}

void finalize_clear_grid_visitor() {
    destroy_clear_grid_visitor(&clear_grid_visitor);
}

void initialize_clear_grid_visitor() {
    construct_clear_grid_visitor(&clear_grid_visitor);
    std::atexit(finalize_clear_grid_visitor);
}
} // namespace nfs::application::pending
NFS_KERNEL_INITIALIZER(startup_initialize_clear_grid_visitor, ".CRT$XCU_NFS_00690298",
                       nfs::application::pending::initialize_clear_grid_visitor);
