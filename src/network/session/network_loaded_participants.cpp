#include "../vehicles/remote_car_runtime.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void finalize_network_load(Dword *owner) {
    Dword *rules = system(72);
    at<Dword>(rules, 168) = 0;
    at<Dword>(rules, 172) = 0;
    at<Dword>(rules, 184) = 0;
    network::packets::pending::set_game_rule(rules, 14, 0);
    at<Dword>(owner, 16) = 0;
    namespace queue = nfs::lifecycle::construction::network::queue;
    queue::visit(network::server_queue(owner), 0, 0,
                 reinterpret_cast<Dword *>(&network::players::prepare), 0, nullptr);
    const int participants = at<int>(rules, 168);
    Dword retained[21]{};
    const unsigned advertised = at<Byte>(owner, 44);
    if (participants != 0 && participants < int(advertised) && advertised < 25) {
        for (unsigned slot = 0; slot < advertised; ++slot) {
            void *record = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
            if (record && at<Dword>(record, 4)) {
                for (unsigned word = 0; word < 21; ++word)
                    retained[word] = at<Dword>(record, word * 4);
                break;
            }
        }
        for (unsigned slot = 0; slot < advertised; ++slot) {
            void *record = slot < 24 ? bytes(system(72)) + 248 + slot * 84 : nullptr;
            if (record && !at<Dword>(record, 4)) {
                for (unsigned word = 0; word < 21; ++word)
                    at<Dword>(rules, 248 + slot * 84 + word * 4) = retained[word];
                at<Dword>(record, 0) = 0;
                break;
            }
        }
        at<Dword>(rules, 168) = advertised;
    }
    at<Dword>(owner, 32) = 1;
    if (at<std::int16_t>(owner, 8) < 4)
        at<std::uint16_t>(owner, 8) = 4;
    at<std::uint16_t>(owner, 10) = at<std::uint16_t>(owner, 8);
    if (at<int>(system(72), 168) < 1)
        network::disconnect_client(network::client(), 16);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
