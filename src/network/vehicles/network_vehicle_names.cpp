#include "nfs/network/session/network_player_state.hpp"
namespace nfs::network::players {
// Table extent/termination beyond valid IDs is unresolved.
const char *vehicle_name(int identifier) {
    Dword index = 0;
    int value = pending::vehicle_names[0].identifier;
    if (value != -1)
        do {
            if (value == identifier)
                break;
            value = pending::vehicle_names[++index].identifier;
        } while (value != -1);
    return pending::vehicle_names[index].name;
}
} // namespace nfs::network::players
