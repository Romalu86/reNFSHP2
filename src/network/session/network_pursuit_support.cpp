#include "network_pursuit_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void decrement_support_statistic(void *record, int kind) {
    if (kind >= 0 && kind < 10) {
        Dword &count = at<Dword>(record, 892 + Dword(kind) * 4);
        --count;
        if (int(count) < 0)
            count = 0;
    }
}

void release_support_request(void *race, Dword participant, int kind) {
    decrement_support_statistic(bytes(race) + participant * 988 + 584, kind);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
namespace nfs::game::runtime::ai::cars {

Dword cop_unavailable(void *cop) {
    return !at<Byte>(cop, 212) || at<int>(cop, 404) == 5 || at<int>(cop, 404) == 4;
}

int idle_roadblock_cop_count(void *owner) {
    Dword remaining = at<std::uint16_t>(owner, 82);
    int available = 0;
    for (unsigned index = 0; remaining; ++index, --remaining)
        if (!at<Dword>(at<void *>(owner, 32 + index * 4), 572))
            ++available;
    return available;
}

Byte free_spike_available(void *pool) {
    for (unsigned index = 0; index < 2; ++index) {
        void *spike = at<void *>(pool, index * 4);
        if (spike && !at<Byte>(spike, 232))
            return 1;
    }
    return 0;
}

void play_unavailable_action(Dword listener) {
    if (at<Dword>(system(72), 140))
        return;
    const Dword pan =
        race_audio::pending::race_audio_listener_count == 2 ? (listener == 0 ? 49152 : 16383) : 0;
    if (listener)
        listener = 256;
    const int mode = at<int>(system(72), 152);
    race_audio::pending::play_event(listener | 114, 9, 64,
                                             mode == 10 || mode == 4 ? 64 : 127, pan, 0);
}
} // namespace nfs::game::runtime::ai::cars
