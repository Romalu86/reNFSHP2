#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
namespace nfs::game::audio {
using namespace state;
// Random first candidate, then the saved linear no-repeat scan.
int choose_track() {
    Library &catalog = library();
    if (catalog.track_count < 1)
        return -1;
    Dword index;
    if (catalog.selection == -2 && sound::field<Dword>(lifecycle::manager->slots[18], 172) &&
        catalog.instrumental_count) {
        index = network::rules::random() % Dword(catalog.instrumental_count) +
                Dword(catalog.first_instrumental);
        if (catalog.played[index] == 1) {
            index = Dword(catalog.first_instrumental);
            while (int(index) <
                   int(Dword(catalog.instrumental_count) + Dword(catalog.first_instrumental))) {
                if (catalog.played[index] != 1) {
                    catalog.played[index] = 1;
                    return int(index);
                }
                ++index;
            }
            for (int clear = catalog.first_instrumental;
                 clear < int(Dword(catalog.instrumental_count) + Dword(catalog.first_instrumental));
                 ++clear)
                catalog.played[clear] = 0;
            index = network::rules::random() % Dword(catalog.instrumental_count) +
                    Dword(catalog.first_instrumental);
        }
    } else {
        if (!catalog.vocal_count)
            return -1;
        index = network::rules::random() % Dword(catalog.vocal_count);
        if (catalog.played[index] == 1) {
            index = 0;
            while (int(index) < catalog.vocal_count) {
                if (catalog.played[index] != 1) {
                    catalog.played[index] = 1;
                    return int(index);
                }
                ++index;
            }
            for (int clear = 0; clear < catalog.vocal_count; ++clear)
                catalog.played[clear] = 0;
            index = network::rules::random() % Dword(catalog.vocal_count);
        }
    }
    catalog.played[index] = 1;
    return int(index);
}
} // namespace nfs::game::audio
