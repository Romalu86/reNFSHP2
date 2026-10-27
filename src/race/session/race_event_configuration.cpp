#include "race_session.hpp"
#include <cstring>
namespace nfs::application::pending {

void enable_catalogue_index(void *catalogue, int index, Byte enabled) {
    at<Byte>(catalogue, 28 + Dword(index) * 1228) = enabled;
}

void enable_catalogue_car(void *catalogue, int car, Byte enabled) {
    for (int i = 0; i < at<int>(catalogue, 0); ++i)
        if (at<int>(catalogue, 4 + Dword(i) * 1228) == car) {
            at<Byte>(catalogue, 28 + Dword(i) * 1228) = enabled;
            return;
        }
}

void set_round_rule_cache(int rule, Dword value) {
    game::runtime::race_rule_cache[rule] = value;
}

void clear_event_progress(void *progress) {
    at<Dword>(progress, 0) &= 4294967054u;
    kernel::memory_zero(bytes(progress) + 308, 192);
    for (unsigned i = 0; i < 12; ++i)
        at<Dword>(progress, 548 + i * 4) = 0;
}

void write_event_track(void *progress, Dword stage, const void *track) {
    auto packed = at<std::uint16_t>(progress, 4 + stage * 2);
    packed ^= (Byte(at<Byte>(track, 0) ^ Byte(packed)) & 31);
    packed ^= (Byte((at<Byte>(track, 4) << 5) ^ Byte(packed)) & 224);
    packed ^= ((std::uint16_t(at<Byte>(track, 8)) << 8) ^ packed) & 3840;
    packed ^= ((std::uint16_t(at<std::uint16_t>(track, 16)) << 12) ^ packed) & 4096;
    packed ^= ((std::uint16_t(at<std::uint16_t>(track, 20)) << 13) ^ packed) & 8192;
    at<std::uint16_t>(progress, 4 + stage * 2) = packed;
}

void write_race_reward_rules(void *rules, const void *settings, const void *rewards) {
    for (unsigned i = 0; i < 18; ++i)
        at<Dword>(rules, 102924 + i * 4) = at<Dword>(settings, i * 4);
    kernel::memory_copy(bytes(rules) + 102996, rewards, 12);
}

void configure_selected_event(void *screen) {
    void *event = at<void *>(screen, 308);
    if (!event)
        return;
    Dword *rules = system(72);
    const Dword stage = at<Dword>(screen, 312);
    at<Dword>(rules, 102920) = stage;
    auto set = [rules](int index, Dword value) {
        network::packets::pending::set_game_rule(rules, index, value);
    };
    set(2, at<Dword>(event, 92));
    set(14, at<Dword>(event, 96));
    set(15, at<Dword>(event, 100) + 37);
    set(16, at<Dword>(event, 104));
    set(3, Dword(static_cast<std::int64_t>(static_cast<long double>(at<int>(event, 124)) * 60.0f)));
    set(11, at<Byte>(event, 4) != 0);
    set(7, at<Dword>(event, 152 + stage * 28));
    set(12, at<Dword>(event, 140) ? 2 : 0);
    set_round_rule_cache(8, at<Dword>(event, 128));
    set_round_rule_cache(7, at<Dword>(event, 132));
    set_round_rule_cache(3, at<Dword>(event, 136));
    void *progress = active_event_progress(rules);
    if (at<Dword>(rules, 152) == 2 && event_complete_progress(progress))
        clear_event_progress(progress);
    for (int i = 0; i < at<int>(event, 116); ++i) {
        void *track = bytes(event) + 144 + Dword(i) * 28;
        if (!i) {
            const Dword id = at<Dword>(track, 0);
            network::packets::pending::select_track(system(40), id);
            set(5, id);
            for (unsigned word = 0; word < 7; ++word)
                at<Dword>(rules, 220 + word * 4) = at<Dword>(track, word * 4);
        }
        if (at<Dword>(rules, 152) == 2) {
            write_event_track(progress, Dword(i), track);
            write_event_helicopters(progress, at<Dword>(event, 140) ? 2 : 0);
        }
    }
    void *catalogue = bytes(system(52)) + 4;
    const int car = at<int>(event, 372);
    enable_catalogue_car(catalogue, car, 1);
    const Dword slot = at<Dword>(rules, 204);
    at<int>(rules, 308 + slot * 84) = car;
    at<Dword>(rules, (slot + 3) * 84) = 0;
    at<Dword>(rules, 312 + slot * 84) = 0;
    for (int i = 0; i < at<int>(catalogue, 0); ++i)
        enable_catalogue_index(catalogue, i, 0);
    for (int i = 0, count = at<int>(event, 368); i < count; ++i)
        enable_catalogue_car(catalogue, at<int>(event, 372 + Dword(i) * 4), 1);
    event_car_order_count = at<Dword>(event, 468);
    for (int i = 0; i < int(event_car_order_count); ++i)
        event_car_order[i] = at<Dword>(event, 472 + Dword(i) * 4);
    write_race_reward_rules(rules, bytes(event) + 8, bytes(event) + 80);
}
} // namespace nfs::application::pending
