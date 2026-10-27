#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/core/console/console.hpp"
#include <cstring>
namespace nfs::lifecycle::table_entries {
using namespace game::runtime;

void __fastcall load_event_lists(System *object, void *) {
    events::clear_subscribers(reinterpret_cast<Dword *>(object));
}

void __fastcall clear_event_lists(System *object, void *) {
    events::clear_subscribers(reinterpret_cast<Dword *>(object));
}

void __fastcall load_rule_network_state(System *object, void *) {
    if (network::session_active()) {
        at<Dword>(object, 204) = 4294967295u;
        at<Dword>(object, 208) = 4294967295u;
    }
}

void __fastcall load_lod_level(System *object, void *) {
    network::rules::pending::reload_lod(reinterpret_cast<Dword *>(object));
}

void __fastcall unload_race_delegates(System *object, void *) {

    if (void *race = at<void *>(object, 4)) {
        invoke(race, 20);
        invoke(race, 33, 1u);
    }
    at<Dword>(object, 4) = 0;
    if (void *delegate = at<void *>(object, 8))
        invoke(delegate, 33, 1u);
    at<Dword>(object, 8) = 0;
}

void __fastcall reset_race_rules(System *object, void *) {
    if (void *race = at<void *>(object, 4))
        invoke(race, 18);
    Dword *rules = system(72);
    cache_rule(rules, 7, cached_rule_7);
    cache_rule(rules, 8, cached_rule_8);
    cache_rule(rules, 3, cached_rule_3);
}
} // namespace nfs::lifecycle::table_entries
namespace nfs::game::runtime {
Dword race_rule_cache[10]{};
Dword &cached_rule_7 = race_rule_cache[7], &cached_rule_8 = race_rule_cache[8],
     &cached_rule_3 = race_rule_cache[3];

void cache_rule(Dword *object, Dword index, Dword value) {
    if (index < 10)
        at<Dword>(object, 2268 + index * 4) = value;
}
} // namespace nfs::game::runtime
