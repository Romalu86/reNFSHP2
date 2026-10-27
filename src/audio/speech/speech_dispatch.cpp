#include "nfs/audio/speech/speech.hpp"
namespace nfs::speech {

void flush_events(Dword voice) {
    if (!has_event_chain(static_cast<int>(voice)))
        return;
    const int slot = state<int[8]>(8768064)[voice];
    reset_utterance();
    const int ready = evaluate_queued_request(slot);
    remove_event_chain(slot);
    if (ready)
        assemble_utterance(static_cast<int>(voice));
}
} // namespace nfs::speech
