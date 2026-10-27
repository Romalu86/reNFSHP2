#include "../../network/session/network_pursuit_runtime.hpp"
#include "../arrest/arrest_cinematic_runtime.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::runtime::ai::cars {
using namespace lifecycle::pending;

void __fastcall autonomous_cop_pursuit_event(void *entity, void *, Dword target_word,
                                             Dword announcement) {
    void *target = reinterpret_cast<void *>(target_word);
    if (!network_chase(target) && !at<Byte>(target, 596))
        allocate_police_chase(&lifecycle::state<Dword>(7353160), entity, target,
                              Byte(announcement));
}

void __fastcall network_car_pursuit_event(void *entity, void *, Dword target_word, Dword) {
    allocate_police_chase(&lifecycle::state<Dword>(7353160), entity,
                          reinterpret_cast<void *>(target_word), 0);
    void *chase = network_chase(entity);
    if (!chase)
        return;
    const int split = at<int>(system(72), 176);
    at<Byte>(chase, 108) = 1;
    if (split == 1 && audio::state::option<Dword>(audio_layout::speech_stream_active) && at<Byte>(system(84), 91))
        return;
    void *target = at<void *>(chase, 44);
    const Dword threat = at<Dword>(entities::interface_at(target, 4), 12);
    const int mode = at<int>(system(72), 152);
    int listener = 0;
    if (mode == 10 || mode == 4) {
        listener = pursuit_listener(chase);
        if (listener == -1)
            listener = 0;
    }
    target = at<void *>(chase, 44);
    const Dword colour = police_colour_mask(at<int>(target, 364));
    void *owner = system(84);
    const Dword model = car_speech_mask(at<int>(target, 356));
    speech::game::dispatch_police_event(owner, 1 + Dword(threat > 1), listener, 0, 0, 0, int(colour),
                                        int(model), 0, 2, 0, 0);
}
} // namespace nfs::game::runtime::ai::cars
