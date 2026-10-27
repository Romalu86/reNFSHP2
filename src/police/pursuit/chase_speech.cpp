#include "police_chase_x87.hpp"
#include "chase_runtime.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
using kernel::numeric::Extended;





Dword *sound_location_marker(void *level, int index) {
    return index < at<int>(level, 108)
               ? reinterpret_cast<Dword *>(bytes(at<void *>(level, 112)) + Dword(index) * 20)
               : nullptr;
}

Dword sound_location(const float *position) {
    return police_frame::sound_location_retained(position);
}

int pursuit_listener(void *chase) {
    if (!chase || !at<Dword>(chase, 40))
        return -1;
    if (at<void *>(chase, 28) == at<void *>(system(76), 84164))
        return 0;
    return at<void *>(chase, 28) == at<void *>(system(76), 84168) ? 1 : -1;
}
int player_chase_listener(void *chase) {
    return at<int>(system(84),
                   at<int>(system(72), 176) == 1 ||
                           at<void *>(chase, 44) == at<void *>(system(76), 84164)
                       ? 64
                       : 68);
}
int player_target_listener(void *target) {
    return at<int>(system(84),
                   at<int>(system(72), 176) == 1 ||
                           target == at<void *>(system(76), 84164)
                       ? 64
                       : 68);
}
// Production callers discard the saved incidental EAX value.
void update_chase_announcements(void *owner, void *chase) {
    police_frame::update_chase_announcements_retained(owner, chase);
}
// Production callers discard the saved incidental EAX value.
void update_helicopter_announcement(void *owner, void *chase) {
    police_frame::update_helicopter_announcement_retained(owner, chase);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
