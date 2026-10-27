#include "race_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::application::pending {

void *construct_event_base(void *race, Dword mode, Dword event) {
    construct_race_state(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_base_methods;
    at<Byte>(race, 34592) = 0;
    at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_paired_event(void *race, Dword mode, Dword event) {
    construct_race_state(race, mode, event);
    at<const RaceMethod *>(race, 0) = paired_event_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_zero(void *race, Dword mode, Dword event) {
    construct_event_base(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_zero_methods;
    at<Byte>(race, 34592) = 0;
    at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_one(void *race, Dword mode, Dword event) {
    construct_event_base(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_one_methods;
    at<Byte>(race, 34592) = 0;
    at<Byte>(race, 34593) = 1;
    at<Dword>(race, 34628) = at<Dword>(race, 34632) = at<Dword>(race, 34636) =
        at<Dword>(race, 34640) = 0;
    return race;
}

void *construct_event_three(void *race, Dword mode, Dword event) {
    construct_event_base(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_three_methods;
    at<Byte>(race, 34592) = 0;
    at<Byte>(race, 34593) = 1;
    at<Dword>(race, 34628) = 0;
    return race;
}

void *construct_event_five(void *race, Dword mode, Dword event) {
    construct_event_base(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_five_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 0;
    return race;
}

void *construct_event_four(void *race, Dword mode, Dword event) {
    construct_paired_event(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_four_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_nine(void *race, Dword mode, Dword event) {
    construct_paired_event(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_nine_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_eleven(void *race, Dword mode, Dword event) {
    construct_event_nine(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_eleven_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_twelve(void *race, Dword mode, Dword event) {
    construct_paired_event(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_twelve_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_thirteen(void *race, Dword mode, Dword event) {
    construct_paired_event(race, mode, event);
    at<const RaceMethod *>(race, 0) = event_thirteen_methods;
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 1;
    return race;
}

void *construct_event_six(void *race, Dword mode, Dword event) {
    construct_paired_event(race, mode, event);
    void *rules = system(72);
    at<Dword>(race, 34584) = at<Dword>(rules, 232);
    const Dword first_player = at<Dword>(rules, 204), second_player = at<Dword>(rules, 208);
    at<Byte>(race, 34592) = at<Byte>(race, 34593) = 0;
    at<const RaceMethod *>(race, 0) = event_six_methods;
    at<Dword>(race, 34488 + first_player * 4) = 0;
    at<Dword>(race, 34488 + second_player * 4) = 0;
    at<Byte>(race, 34632) = at<Byte>(race, 34634) = 0;
    at<Dword>(race, 34628) = Dword(-1);
    at<Byte>(race, 34633) = 1;
    for (unsigned index = 0; index < 24; ++index)
        at<Dword>(race, 34488 + index * 4) = 0;
    return race;
}
namespace {
using Construct = void *(*)(void *, Dword, Dword);
Dword allocate_event(Dword mode, Dword event, const char *name, Dword pool, bool allow_six,
                    bool allow_four) {
    Construct construct = construct_event_base;
    int bytes = 34628;
    const RaceMethod *override_methods = nullptr;
    switch (event) {
    case 0:
        construct = construct_event_zero;
        break;
    case 1:
        construct = construct_event_one;
        bytes = 34644;
        break;
    case 2:
        override_methods = event_two_methods;
        break;
    case 3:
        construct = construct_event_three;
        bytes = 34632;
        break;
    case 4:
        if (allow_four)
            construct = construct_event_four;
        break;
    case 5:
        construct = construct_event_five;
        break;
    case 6:
        if (allow_six) {
            construct = construct_event_six;
            bytes = 34636;
        }
        break;
    case 7:
        override_methods = event_seven_methods;
        break;
    case 9:
        construct = construct_event_nine;
        break;
    case 10:
        construct = construct_event_four;
        break;
    case 11:
        construct = construct_event_eleven;
        break;
    case 12:
        construct = construct_event_twelve;
        break;
    case 13:
        construct = construct_event_thirteen;
        break;
    }
    void *race = lifecycle::allocate(bytes, name, pool);
    if (!race)
        return 0;
    try {
        construct(race, mode, event);
    } catch (...) {
        game::runtime::track::pending::release_named(race, name, pool);
        throw;
    }
    if (override_methods)
        at<const RaceMethod *>(race, 0) = override_methods;
    return reinterpret_cast<Dword>(race);
}
} // namespace

Dword create_standard_race(Dword mode, Dword event, const char *name, Dword pool) {
    return allocate_event(mode, event, name, pool, true, true);
}

Dword create_pursuit_race(Dword mode, Dword event, const char *name, Dword pool) {
    return allocate_event(mode, event, name, pool, false, true);
}

Dword create_mode_three_race(Dword mode, Dword event, const char *name, Dword pool) {
    return allocate_event(mode, event, name, pool, false, true);
}

Dword create_championship_race(Dword mode, Dword event, const char *name, Dword pool) {
    return allocate_event(mode, event, name, pool, false, false);
}

Dword create_mode_five_race(Dword mode, Dword event, const char *name, Dword pool) {
    return allocate_event(mode, event, name, pool, false, false);
}

Dword create_race(Dword event, Dword mode, const char *name, Dword pool) {
    switch (mode) {
    case 2:
        return create_pursuit_race(2, event, name, pool);
    case 3:
        return create_mode_three_race(3, event, name, pool);
    case 4:
        return create_championship_race(4, event, name, pool);
    case 5:
        return create_mode_five_race(5, event, name, pool);
    default:
        return create_standard_race(mode, event, name, pool);
    }
}
} // namespace nfs::application::pending
