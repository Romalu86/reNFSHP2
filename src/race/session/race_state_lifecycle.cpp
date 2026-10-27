#include "race_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::pending {
namespace {
void construct_participants(void *race) {
    lifecycle::construction::construct_array(bytes(race) + 584, 988, 24,
                                             construct_participant,
                                             network::world::destroy_race_player);
}
void destroy_participants(void *race) {
    lifecycle::construction::objects::destroy_array(bytes(race) + 584, 988, 24,
                                                    network::world::destroy_race_player);
}
void copy_state_header(void *race, const void *source) {
    const Dword mode = at<Dword>(source, 34616);
    const Dword event = at<Dword>(source, 34620);
    at<Dword>(race, 34616) = mode;
    at<Dword>(race, 34612) = at<Dword>(source, 34612);
    at<Dword>(race, 34604) = at<Dword>(source, 34604);
    const Dword deadline = at<Dword>(source, 34596);
    at<Dword>(race, 34620) = event;
    const Dword elapsed = at<Dword>(source, 34600);
    at<Dword>(race, 34596) = deadline;
    const Byte local = at<Byte>(source, 34592);
    at<Dword>(race, 34600) = elapsed;
    const Dword counter = at<Dword>(source, 34608);
    at<Byte>(race, 34592) = local;
    at<Dword>(race, 34608) = counter;
    at<Dword>(race, 34624) = at<Dword>(source, 34624);
    at<Byte>(race, 34593) = at<Byte>(source, 34593);
}
void copy_state_payload(void *race, const void *source) {
    for (unsigned lap = 0; lap < 100; ++lap) {
        for (unsigned slot = 0; slot < 24; ++slot)
            at<Dword>(race, 24296 + (lap * 24 + slot) * 4) =
                at<Dword>(source, 24296 + (lap * 24 + slot) * 4);
        at<Dword>(race, 33896 + lap * 4) = at<Dword>(source, 33896 + lap * 4);
    }
    const Dword counter = at<Dword>(source, 48);
    for (unsigned i = 0; i < 10; ++i)
        at<Dword>(race, 4 + i * 4) = at<Dword>(source, 4 + i * 4);
    at<Dword>(race, 48) = counter;
    for (unsigned i = 0; i < 24; ++i)
        assign_participant(participant(race, i), bytes(source) + 584 + i * 988);
    alignas(4) Byte rules[72], rewards[12];
    read_race_reward_rules(system(72), rules, rewards);
    for (unsigned i = 0; i < 24; ++i)
        at<Dword>(race, 34488 + i * 4) = at<Dword>(source, 34488 + i * 4);
}
} // namespace

void *construct_race_state(void *race, Dword mode, Dword event) {
    initialize_race_rewards(bytes(race) + 164);
    construct_participants(race);
    try {
        at<const RaceMethod *>(race, 0) = race_state_methods;
        auto *signal = reinterpret_cast<kernel::Signal *>(bytes(race) + 156);
        if (kernel::signal_init(signal))
            kernel::signal_set(signal);
        at<Dword>(race, 34616) = mode;
        at<Dword>(race, 148) = 0;
        at<Dword>(race, 34620) = event;
        at<Dword>(race, 34612) = 0;
        at<Dword>(race, 34600) = 0;
        at<Dword>(race, 34604) = 0;
        at<Dword>(race, 34608) = 0;
        at<Dword>(race, 34596) = Dword(-1);
        at<Dword>(race, 34624) = 0;
        at<Byte>(race, 34592) = 0;
        at<Byte>(race, 34593) = 0;
        at<Byte>(race, 44) = 0;
        for (unsigned i = 0; i < 10; ++i)
            at<Dword>(race, 4 + i * 4) = 9;
        void *rules = system(72);
        at<Dword>(race, 48) = 0;
        if (at<Dword>(rules, 152) == 2 && at<Byte>(race, 44) && !at<Dword>(rules, 140))
            mark_event_entered(active_event_progress(rules));
        at<Dword>(race, 34600) = 0;
        invoke(race, 17);
        alignas(4) Byte settings[72], rewards[12];
        read_race_reward_rules(system(72), settings, rewards);
        load_race_reward_rules(bytes(race) + 164, rewards);
        for (unsigned i = 0; i < 24; ++i) {
            at<Dword>(race, 34488 + i * 4) = 0;
            participant(race, i)->disabled = 0;
        }
    } catch (...) {
        destroy_participants(race);
        throw;
    }
    return race;
}

void destroy_race_state(void *race) {
    at<const RaceMethod *>(race, 0) = race_state_methods;
    try {
        kernel::signal_destroy(reinterpret_cast<kernel::Signal *>(bytes(race) + 156));
    } catch (...) {
        destroy_participants(race);
        throw;
    }
    destroy_participants(race);
}
// The saved copy constructor does not create a signal.
void *copy_construct_race_state(void *race, const void *source) {
    initialize_race_rewards(bytes(race) + 164);
    construct_participants(race);
    try {
        copy_state_header(race, source);
        at<const RaceMethod *>(race, 0) = race_state_methods;
        copy_state_payload(race, source);
    } catch (...) {
        destroy_participants(race);
        throw;
    }
    return race;
}

void *copy_race_state(void *race, const void *source) {
    if (race != source) {
        copy_state_header(race, source);
        copy_state_payload(race, source);
    }
    return race;
}

void *copy_event_base(void *race, const void *source) {
    copy_race_state(race, source);
    return race;
}

void *copy_paired_event(void *race, const void *source) {
    copy_race_state(race, source);
    return race;
}

void __fastcall reset_race_statistics(void *race, void *) {
    at<volatile Dword>(race, 34612) = 0;
    at<volatile Dword>(race, 34600) = 0;
    at<volatile Dword>(race, 34604) = 0;
    for (unsigned lap = 0; lap < 100; ++lap) {
        for (unsigned slot = 0; slot < 24; ++slot)
            at<volatile Dword>(race, 24296 + (lap * 24 + slot) * 4) = Dword(-1);
        at<volatile Dword>(race, 33896 + lap * 4) = Dword(-1);
    }
    for (unsigned i = 0; i < 24; ++i)
        reset_participant(participant(race, i));
    at<volatile Dword>(race, 34596) = Dword(-1);
    invoke(race, 8);
}
} // namespace nfs::application::pending
