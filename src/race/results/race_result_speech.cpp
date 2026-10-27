#include "../session/race_session.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/audio/speech/speech.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::pending {

void announce_race_results(void *owner, int first_place, int second_place) {
    if (!at<Dword>(owner, 20))
        return;
    using namespace game::audio;
    state::configuration<Dword>(audio_layout::player_volume) = 0;
    state::configuration<Dword>(audio_layout::opponent_volume) = 0;
    state::configuration<Dword>(12) = 1;
    apply_event_volumes();
    stop_race_channels();
    sound::streaming::stop_stream(at<Dword>(owner, 16));
    at<Dword>(owner, 28) = 0;
    auto say = [](Dword event) {
        speech::event_sink(speech::create_event_code(0, 0, event), 0);
        speech::flush_events(0);
    };
    auto masked = [](Dword event, Dword mask, bool two_arguments) {
        const Dword code = speech::create_event_code(0, 0, event);
        if (two_arguments)
            speech::event_sink(code, 2, mask, 0);
        else
            speech::event_sink(code, 1, mask);
        speech::flush_events(0);
    };
    const int event = at<int>(system(72), 152);
    if (at<int>(system(72), 176) == 2) {
        const int winner = second_place < first_place ? 2 : 1;
        const int winning_place = second_place < first_place ? second_place : first_place;
        switch (event) {
        case 1:
            say(winning_place == 1 ? 169 : 220);
            return;
        case 6:
            masked(213, winner == 1 ? 131072 : 32768, false);
            return;
        case 4:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            break;
        default:
            if (first_place < at<int>(system(68), 8)) {
                const Dword mask = first_place == 1   ? 2
                                  : first_place == 2 ? 16
                                  : first_place == 3 ? 128
                                                     : 0;
                masked(165, mask, true);
            }
            if (second_place < at<int>(system(68), 8)) {
                const Dword mask = second_place == 1   ? 4
                                  : second_place == 2 ? 32
                                  : second_place == 3 ? 256
                                                      : 0;
                masked(165, mask, true);
            }
        }
        return;
    }
    if (services::special_race_mode(system(72)) & 255) {
        if (first_place == 1 || first_place == 2 || first_place == 3)
            masked(167, first_place == 1 ? 2 : first_place == 2 ? 8 : 32, false);
        return;
    }
    if (at<int>(system(68), 8) == 1)
        return;
    switch (event) {
    case 1:
        say(first_place == 1 ? 169 : 220);
        return;
    case 4:
    case 6:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
        break;
    default:
        if (first_place < at<int>(system(68), 8)) {
            const Dword mask = first_place == 1   ? 1
                              : first_place == 2 ? 8
                              : first_place == 3 ? 64
                                                 : 0;
            masked(165, mask, true);
        }
    }
}
} // namespace nfs::application::pending
