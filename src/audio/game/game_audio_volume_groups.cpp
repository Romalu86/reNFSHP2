#include "nfs/audio/music/music_clients.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void apply_event_volumes() {
    int opponent = configuration<int>(audio_layout::opponent_volume), track = configuration<int>(audio_layout::track_volume),
        loop = configuration<int>(audio_layout::boom_loop_volume);
    for (int listener = 0; listener < 2; ++listener) {
        EventChannel *events = channels() + listener * 124;
        auto set = [&](int first, int last, int volume) {
            for (int id = first; id <= last; ++id)
                events[id].volume = volume;
        };
        set(0, 2, 110);
        set(3, 6, configuration<int>(audio_layout::player_skid_volume));
        set(7, 7, configuration<int>(audio_layout::car_collision_volume));
        set(8, 9, track);
        set(10, 10, 127);
        set(11, 13, track);
        set(14, 17, configuration<int>(audio_layout::traffic_volume));
        set(18, 21, 0);
        set(22, 57, opponent);
        set(58, 61, 0);
        set(62, 77, opponent);
        set(78, 81, 120);
        set(82, 105, loop);
        set(106, 112, 0);
        set(113, 113, track);
        set(114, 114, 127);
        set(115, 115, 0);
        set(116, 116, configuration<int>(audio_layout::car_collision_volume));
        set(117, 117, configuration<int>(audio_layout::boom_oneshot_volume));
        set(118, 119, configuration<int>(audio_layout::boom_collision_volume));
        set(120, 123, 0);
    }
    if (configuration<int>(audio_layout::player_volume) < 0)
        configuration<int>(audio_layout::player_volume) = configuration<int>(audio_layout::baseline_player_volume);
    configuration<int>(audio_layout::car_volume_limit) = configuration<int>(audio_layout::player_volume);
    if (configuration<int>(audio_layout::second_player_volume) < 0)
        configuration<int>(audio_layout::second_player_volume) = configuration<int>(audio_layout::baseline_second_player_volume);
}
} // namespace nfs::game::audio
