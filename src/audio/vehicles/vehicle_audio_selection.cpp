#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::race_audio::pending {
namespace v = game::audio::vehicle;
namespace state = game::audio::state;

void update_surfaces(Dword requested_count) {
    int count = int(requested_count);
    struct Candidate {
        void *car;
        int distance;
        Dword reserved[4], environment;
    };
    Candidate nearest[4];
    for (int i = 0; i < 4; ++i) {
        nearest[i].car = nullptr;
        nearest[i].distance = 100000;
    }
    int listener = listener_index;
    for (int i = 0; i < at<int>(listener_state, 220 + listener_index * 2760);
         ++i) {
        Byte *entry = listener_state + 224 + listener_index * 2760 + i * 16;
        void *car = at<void *>(entry, 0);
        Dword environment = at<Dword>(entry, 12);
        if (v::car_audio_suppressed(car) || !at<Byte>(at<void *>(car, 1768), 728))
            continue;
        int distance = at<int>(listener_state,
                               232 + listener_index * 2760 + i * 16);
        for (int rank = 0; rank < count; ++rank) {
            if (nearest[rank].distance <= distance)
                continue;
            for (int tail = count - 1; tail > rank; --tail) {
                nearest[tail].car = nearest[tail - 1].car;
                nearest[tail].distance = nearest[tail - 1].distance;
                nearest[tail].environment = nearest[tail - 1].environment;
            }
            nearest[rank].car = car;
            nearest[rank].distance = distance;
            nearest[rank].environment = environment;
            break;
        }
    }
    int used[4];
    for (int i = 0; i < count; ++i)
        used[i] = 0;
    listener = listener_index;
    state::world<int>(352 + listener * 2760) = 0;
    for (int rank = 0; rank < count; ++rank) {
        void *car = nearest[rank].car;
        int slot = -1;
        for (int i = 0; i < count; ++i)
            if (car &&
                car == at<void *>(listener_state, 108 + listener * 2760 + i * 28)) {
                used[i] = 1;
                slot = i;
            }
        if (slot == -1)
            for (int i = 0; i < count; ++i)
                if (!used[i] && car) {
                    slot = i;
                    used[i] = 1;
                    break;
                }
        if (slot != -1) {
            Byte *entry = listener_state + 108 + listener * 2760 + slot * 28;
            at<void *>(entry, 0) = car;
            at<Dword>(entry, 24) = nearest[rank].environment;
            at<int>(entry, 4) = nearest[rank].distance;
            at<int>(entry, 8) = v::wrapped_product(nearest[rank].distance, nearest[rank].distance);
            at<int>(entry, 12) = slot;
        }
    }
    for (int i = 0; i < count; ++i) {
        listener = listener_index;
        if (!used[i]) {
            at<void *>(listener_state, 108 + listener * 2760 + i * 28) = nullptr;
            if (engine_active(i, listener))
                stop_engine(i, listener_index);
        } else
            ++state::world<int>(352 + listener * 2760);
    }
}
} // namespace nfs::game::runtime::race_audio::pending
