#include "../session/race_session.hpp"
#include "../rules/race_rules_x87.hpp"
namespace nfs::application::pending {
namespace {
void *actor_interface(Dword value, unsigned slot) {
    void *object = reinterpret_cast<void *>(value);
    const int *offsets = at<const int *>(object, 4);
    return invoke<void *>(bytes(object) + 4 + offsets[2], slot);
}
} // namespace

__declspec(noinline) float load_participant_speed(const volatile float *source) {
    return *source;
}

void set_actor_finished(void *actor, Byte finished) {
    at<Byte>(actor, 409) = finished;
    at<Dword>(actor, 412) = finished ? race_clock() : Dword(-1);
}

void *traffic_actor_view(void *actor) {
    return game::runtime::ai::kind(actor) == 3 ? actor : nullptr;
}

void record_participant_speeds(void *record, Dword average_speed_bits, Dword maximum_speed_bits) {
    at<Dword>(record, 48) = average_speed_bits;
    at<Dword>(record, 44) = maximum_speed_bits;
}
// Both random multipliers are binary32 loads in the instructions.
void capture_participant_finish(void *race, Dword slot) {
    if (void *vehicle = network::packets::pending::find_race_vehicle(int(slot)))
        at<Byte>(vehicle, 596) = 1;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    for (Dword value = network::world::first(&iterator); value;
         value = network::world::next(&iterator)) {
        void *actor = actor_interface(value, 1);
        if (!actor || network::world::vehicle_slot(actor) != slot)
            continue;
        void *car = at<void *>(actor, 368);
        Dword maximum_bits = at<Dword>(car, 1852);
        if (!game::runtime::ai::lifecycle::pending::participant_enabled(race, slot)) {
            const Dword random = network::rules::random() & 65535;
            float maximum;
            std::memcpy(&maximum, &maximum_bits, sizeof(maximum));
            const auto factor = static_cast<long double>(random) * 0.0000152587890625f;
            const auto scaled = static_cast<long double>(maximum) * 0.1500000059604644775390625f;
            const float adjusted = float(static_cast<long double>(maximum) - factor * scaled);
            std::memcpy(&maximum_bits, &adjusted, sizeof(maximum_bits));
        }
        // The maximum snapshot is a raw DWORD; only the average has this F32 conversion.
        const float stored_average = load_participant_speed(&at<float>(car, 1856));
        Dword average_bits;
        std::memcpy(&average_bits, &stored_average, sizeof(average_bits));
        record_participant_speeds(participant(race, slot), average_bits, maximum_bits);
    }
}

void mark_ai_retired(void *, Dword slot) {
    using game::runtime::ai::lifecycle::state;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, state<Dword *>(7353312));
    Dword value = network::world::first(&iterator);
    while (!network::world::iterator_done(&iterator)) {
        void *actor = actor_interface(value, 1);
        if (actor && network::world::vehicle_slot(actor) == slot) {
            if (invoke<Dword>(reinterpret_cast<void *>(value), 9)) {
                at<Byte>(actor, 408) = 1;
                invoke(actor, 14, Dword(1));
            }
            return;
        }
        value = network::world::next(&iterator);
    }
}

void finish_participant(void *race, Dword slot) {
    using game::runtime::ai::lifecycle::state;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, state<Dword *>(7353312));
    Dword value = network::world::first(&iterator);
    while (!network::world::iterator_done(&iterator)) {
        void *actor = actor_interface(value, 1);
        if (!actor || network::world::vehicle_slot(actor) != slot) {
            value = network::world::next(&iterator);
            continue;
        }
        if (void *vehicle = actor_interface(value, 2))
            at<Byte>(vehicle, 596) = 1;
        capture_participant_finish(race, slot);
        void *object = reinterpret_cast<void *>(value);
        if (game::runtime::ai::kind(object) == 1 || game::runtime::ai::kind(object) == 2)
            set_actor_finished(actor, 1);
        else {
            if (game::runtime::ai::kind(object) == 3) {
                void *traffic = traffic_actor_view(actor);
                if (!traffic)
                    return;
                at<Byte>(traffic, 1804) = 1;
                return;
            }
            invoke(actor, 14, Dword(2147483647));
        }
        RaceParticipant *record = participant(race, slot);
        if (!network::session_active()) {
            record->finish_time = invoke<int>(race, 22);
            return;
        }
        if ((!participant_has_finished(record) && !record->disconnected) || invoke<Byte>(race, 4)) {
            const Dword checkpoint = record->last_checkpoint < 0 ? 0 : Dword(record->last_checkpoint);
            const int time = record->checkpoint_times[checkpoint];
            if (static_cast<int>(checkpoint) >= 0 && time > 0 &&
                (at<int>(system(72), 228) <= static_cast<int>(checkpoint + 1) ||
                 (at<Dword>(race, 34620) == 1 && !participant_has_finished(record) &&
                  !record->disconnected))) {
                record->finish_time = time;
                return;
            }
            record->finish_time = invoke<int>(race, 22);
        }
        return;
    }
}
} // namespace nfs::application::pending
