#include "../../frontend/flow/frontend_simulation.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/audio/speech/speech.hpp"
namespace nfs::application::frontend {
// Callers ignore the return value.
void play_countdown_audio(void *owner, int phase) {
    if (phase != -1) {
        game::runtime::race_audio::pending::play_event(114, phase ? 2 : 3, 64, 127, 0, 0);
        return;
    }
    using game::runtime::at;
    void *rules = game::runtime::system(72);
    const int kind = at<int>(rules, 152);
    if ((kind != 10 && kind != 4) || !at<Dword>(owner, 20) || at<Dword>(rules, 140))
        return;
    auto say = [](Dword event) {
        speech::event_sink(speech::create_event_code(0, 0, event), 0);
        speech::flush_events(0);
    };
    if (network::rules::random() % 100 > 50)
        say(222);
    speech::event_sink(speech::create_event_code(0, 0, 203), 2, 0, 0);
    speech::flush_events(0);
    if (network::rules::random() % 100 > 50)
        say(222);
}
} // namespace nfs::application::frontend
