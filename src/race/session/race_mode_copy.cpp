#include "race_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::application::pending {
// Separate saved assignment entry; callers remain unqualified.
void *copy_event_six(void *race, const void *source) {
    copy_paired_event(race, source);
    const Dword score = at<Dword>(source, 34628);
    const Byte finished = at<Byte>(source, 34632);
    const Byte announced = at<Byte>(source, 34633);
    at<Dword>(race, 34628) = score;
    const Byte changed = at<Byte>(source, 34634);
    at<Byte>(race, 34634) = changed;
    at<Byte>(race, 34632) = finished;
    at<Byte>(race, 34633) = announced;
    return race;
}

void *__fastcall clone_event_eleven(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject0", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_eleven(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject0", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_paired_event(copy, race);
    return copy;
}

void *__fastcall clone_event_six(void *race, void *) {
    void *copy = lifecycle::allocate(34636, "clonedQuotaObject1", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_six(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject1", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_paired_event(copy, race);
    const Dword score = at<Dword>(race, 34628);
    const Byte announced = at<Byte>(race, 34633);
    const Byte finished = at<Byte>(race, 34632);
    at<Dword>(copy, 34628) = score;
    const Byte changed = at<Byte>(race, 34634);
    at<Byte>(copy, 34633) = announced;
    at<Byte>(copy, 34634) = changed;
    at<Byte>(copy, 34632) = finished;
    return copy;
}

void *__fastcall clone_event_base(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject2", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_base(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject2", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_race_state(copy, race);
    return copy;
}

void *__fastcall clone_event_twelve(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject3", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_twelve(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject3", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_paired_event(copy, race);
    return copy;
}

void *__fastcall clone_event_five(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject4", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_five(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject4", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_event_base(copy, race);
    return copy;
}

void *__fastcall clone_event_thirteen(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject5", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_thirteen(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject5", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_paired_event(copy, race);
    return copy;
}

void *__fastcall clone_event_one(void *race, void *) {
    void *copy = lifecycle::allocate(34644, "clonedQuotaObject6", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_one(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject6", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_event_base(copy, race);
    at<Dword>(copy, 34628) = at<Dword>(race, 34628);
    return copy;
}

void *__fastcall clone_event_seven(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject7", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_base(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
            at<const RaceMethod *>(copy, 0) = event_seven_methods;
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject7", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_event_base(copy, race);
    return copy;
}

void *__fastcall clone_event_nine(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject8", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_nine(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject8", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_paired_event(copy, race);
    return copy;
}

void *__fastcall clone_event_four(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObject9", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_four(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObject9", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_paired_event(copy, race);
    return copy;
}

void *__fastcall clone_event_zero(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObjecta", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_zero(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObjecta", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_event_base(copy, race);
    return copy;
}

void *__fastcall clone_paired_event(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObjectb", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_paired_event(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObjectb", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_race_state(copy, race);
    return copy;
}

void *__fastcall clone_event_three(void *race, void *) {
    void *copy =
        lifecycle::allocate(34632, "clonedTimeTrialObject", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_three(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedTimeTrialObject", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_event_base(copy, race);
    at<Dword>(copy, 34628) = at<Dword>(race, 34628);
    return copy;
}

void *__fastcall clone_event_two(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedQuotaObjectd", lifecycle::level_pool | 256);
    if (copy) {
        try {
            construct_event_base(copy, at<Dword>(race, 34616), at<Dword>(race, 34620));
            at<const RaceMethod *>(copy, 0) = event_two_methods;
        } catch (...) {
            game::runtime::track::pending::release_named(
                copy, "clonedQuotaObjectd", lifecycle::level_pool | 256);
            throw;
        }
    }
    copy_race_state(copy, race);
    return copy;
}

void *__fastcall clone_race_state(void *race, void *) {
    void *copy = lifecycle::allocate(34628, "clonedRaceObject", lifecycle::level_pool);
    if (!copy)
        return nullptr;
    try {
        copy_construct_race_state(copy, race);
    } catch (...) {
        game::runtime::track::pending::release_named(
                copy, "clonedRaceObject", lifecycle::level_pool);
        throw;
    }
    return copy;
}
} // namespace nfs::application::pending
