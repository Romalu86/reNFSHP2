#include "../session/race_session.hpp"
namespace nfs::application::pending {
namespace {
void *profile_at_kind(Dword &index, Dword kind) {
    using game::runtime::ai::lifecycle::state;
    while (index < state<Dword>(7353148)) {
        void *profile = state<void *>(7352968 + index * 4);
        if (!profile || invoke<Dword>(profile, 0) == kind)
            return profile;
        ++index;
    }
    return nullptr;
}
} // namespace DIV by zero when no eligible profile is retained from the path.
 
Dword choose_pursuit_grid_entry(void *, Dword, Dword) {
    using game::runtime::ai::lifecycle::state;
    Dword index = 0, count = 0;
    void *profile = profile_at_kind(index, 5);
    while (profile) {
        if (invoke<void *>(profile, 6))
            ++count;
        if (index >= state<Dword>(7353148))
            break;
        ++index;
        profile = profile_at_kind(index, 5);
    }
    const Dword selected = network::rules::random() % count;
    index = 0;
    Dword ordinal = 0;
    profile = profile_at_kind(index, 5);
    while (profile) {
        if (invoke<void *>(profile, 6)) {
            if (ordinal == selected)
                return invoke<Dword>(profile, 12);
            ++ordinal;
        }
        if (index >= state<Dword>(7353148))
            break;
        ++index;
        profile = profile_at_kind(index, 5);
    }
    return Dword(-1);
}
// Count uses all nonnull slot2 results; bounds filter is only in the selection
// pass.
Dword choose_racer_grid_entry(void *, Dword first, Dword last) {
    using game::runtime::ai::lifecycle::state;
    Dword index = 0, count = 0;
    void *profile = profile_at_kind(index, 0);
    while (profile) {
        if (invoke<void *>(profile, 2))
            ++count;
        if (index >= state<Dword>(7353148))
            break;
        ++index;
        profile = profile_at_kind(index, 0);
    }
    const Dword selected = count ? network::rules::random() % count : 0;
    first = std::uint16_t(first);
    last = std::uint16_t(last);
    index = 0;
    Dword ordinal = 0;
    profile = profile_at_kind(index, 0);
    while (profile) {
        void *entry = invoke<void *>(profile, 2);
        if (entry) {
            const Dword rank = at<std::uint16_t>(entry, 160);
            if (first <= rank && rank <= last) {
                if (ordinal == selected)
                    return invoke<Dword>(profile, 12);
                ++ordinal;
            }
        }
        if (index >= state<Dword>(7353148))
            break;
        ++index;
        profile = profile_at_kind(index, 0);
    }
    return Dword(-1);
}
} // namespace nfs::application::pending
