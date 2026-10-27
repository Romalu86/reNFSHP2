#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"

namespace nfs::game::runtime::ai::lifecycle {
namespace {
Dword load_elapsed() {
    return world::pending::legacy_clock_entry() - pending::startup_ticks;
}
struct LoadingProgress {
    audio::vehicle::Progress progress;
    explicit LoadingProgress(int count) {
        audio::vehicle::construct_progress(&progress, count);
    }
    ~LoadingProgress() {
        audio::vehicle::destroy_progress(&progress);
    }
    void advance() {
        audio::vehicle::advance_progress(&progress);
    }
};
Dword *next_traffic_profile(Dword &index, bool include_current) {
    if (!include_current)
        ++index;
    for (; index < state<Dword>(7353148); ++index) {
        Dword *profile = state<Dword *>(7352968 + index * 4);
        if (!profile || invoke<int>(profile, 0) == 4)
            return profile;
    }
    return nullptr;
}
} // namespace

void load_level(Dword *owner) {
    pending::read_race_tuning(owner);
    Dword *connection = system(104);
    reset_pursuit_schedule(reinterpret_cast<Dword *>(bytes(owner) + 84344));
    if (network::session_active() && at<Dword>(connection, 12) == 0) {
        if (at<std::int16_t>(connection, 10) < 4 && at<Dword>(system(104), 16) == 0) {
            pending::load_tick_4 = load_elapsed();
            pending::load_tick_10 = load_elapsed();
            pending::load_tick_end = load_elapsed();
            return;
        }
        pending::finalize_network_load(connection);
    }
    pending::initialize_path_population(ai::pending::path_controller);
    Dword *rules = system(72);
    state<Dword>(7353320) = at<Dword>(rules, 152);
    state<Dword>(7353324) = at<Dword>(rules, 148);
    const int traffic_count = at<int>(rules, 184);
    const int initial_players = at<int>(rules, 176);
    LoadingProgress progress(static_cast<int>(at<Dword>(rules, 168) + traffic_count));
    state<Byte>(7352952) = at<Dword>(rules, 212) != 0;
    Dword *car_manager = system(68);
    if (!network::session_active())
        for (int player = initial_players; player < 2; ++player)
            if (player >= 0 && player < 2)
                at<Dword>(rules, 204 + player * 4) = 4294967295u;
    int local_player = 0, local_racers = 0;
    bool has_remote = false, has_special_racer = false, has_tick_10 = false,
         network_announced = false;
    int car_choices[20];
    for (Dword index = 0; index < at<Dword>(rules, 168); ++index) {
        progress.advance();
        if (index < 15)
            car_choices[index + 1] = static_cast<int>(load_elapsed());
        if (index == 4)
            pending::load_tick_4 = load_elapsed();
        else if (index == 10) {
            pending::load_tick_10 = load_elapsed();
            has_tick_10 = true;
        }
        Dword *descriptor =
            index < 24 ? reinterpret_cast<Dword *>(bytes(rules) + 248 + index * 84) : nullptr;
        int player = -1;
        if (descriptor[0]) {
            if (descriptor[2])
                has_remote = true;
            if (!descriptor[1])
                player = local_player++;
            if (descriptor[16] == 16)
                has_special_racer = true;
        }
        if (descriptor[2]) {
            pending::spawn_remote_car(owner, index, player, descriptor[15], 5, descriptor[3] != 0);
        } else if (descriptor[1]) {
            pending::spawn_player(owner, index, descriptor[15], descriptor[16]);
        } else if (descriptor[0]) {
            pending::spawn_racer(owner, index, player, descriptor[15], descriptor[16]);
            ++local_racers;
        } else if (!network::session_active() && at<Dword>(system(108), 44) == 0) {
            pending::spawn_cop(owner, index, descriptor[13], descriptor[15], has_remote,
                               has_special_racer);
        } else if (at<Dword>(system(108), 44) == 0 && !network_announced) {
            network::disconnect_client(system(112), 16);
            network_announced = true;
        }
    }
    if (!has_tick_10)
        pending::load_tick_10 = load_elapsed();
    pending::load_helicopter_population(ai::collision_state(), local_racers);
    pending::reset_path_population(ai::pending::path_controller);
    const Dword available = static_cast<Dword>(
        selectable_car_count(reinterpret_cast<Dword *>(bytes(system(52)) + 4)));
    Dword profile_index = 0;
    Dword *prototype = next_traffic_profile(profile_index, true);
    int used = 0;
    bool large_traffic = false, second_large_traffic = false;
    for (int traffic = 0; prototype && traffic < traffic_count; ++traffic) {
        progress.advance();
        const Dword profile_data = invoke<Dword>(prototype, 10);
        auto *memory = static_cast<Dword *>(gui::application_allocate(624));
        Dword *entity;
        try {
            entity = memory ? pending::construct_traffic_car(memory, profile_data, 1) : nullptr;
        } catch (...) {
            gui::application_release(memory);
            throw;
        }
        int kind;
        for (;;) {
            const Dword ordinal = emitter_random_word() % available;
            Dword *catalog = reinterpret_cast<Dword *>(bytes(system(52)) + 4);
            kind = selectable_car(catalog, int(ordinal));
            if (large_traffic && (kind == 28 || kind == 29))
                continue;
            if (second_large_traffic && (kind == 26 || kind == 27))
                continue;
            int previous = 0;
            while (previous < used && car_choices[previous] != kind)
                ++previous;
            if (previous == used)
                break;
        }
        if (kind == 28 || kind == 29)
            large_traffic = true;
        if (kind == 26 || kind == 27)
            second_large_traffic = true;
        car_choices[used++] = kind;
        const Dword car_kind = pending::select_car_kind(kind);
        entity[89] = car_kind;
        Dword *catalog = reinterpret_cast<Dword *>(bytes(system(52)) + 4);
        const Dword variants = car_variant_count(catalog, kind);
        const Dword variant = emitter_random_word() % variants;
        entity[90] = pending::parse_car_variant(variant);
        const auto *name = reinterpret_cast<const char *>(pending::car_kind_name(car_kind));
        invoke(entity, 6, Dword{1});
        pending::attach_pursuit_car(reinterpret_cast<Dword *>(bytes(owner) + 84344), entity);
        void *car = world::create_car(car_manager, name, entity[90],
                                      reinterpret_cast<Dword>(entity), 4, 1, 4294967295u);
        void *physics = at<void *>(car, 1768);
        at<Byte>(physics, 728) = at<Byte>(physics, 729) = 0;
        prototype = next_traffic_profile(profile_index, false);
        if (!prototype) {
            profile_index = 0;
            prototype = next_traffic_profile(profile_index, true);
        }
    }
    pending::load_tick_end = load_elapsed();
    pending::finish_population_load(&state<Dword>(7353280));
    begin_path_level();
    Byte loading = 1;
    for (int player = 0; player < 2; ++player) {
        void *entity = at<void *>(owner, 84164 + player * 4);
        if (!entity)
            continue;
        void *base = bytes(entity) + 4 + at<int>(reinterpret_cast<void *>(at<Dword>(entity, 4)), 4);
        if (invoke<int>(base, 0) == 2) {
            loading = 0;
            break;
        }
    }
    state<Byte>(7353316) = 0;
    pending::population_loading = loading;
}
} // namespace nfs::game::runtime::ai::lifecycle
