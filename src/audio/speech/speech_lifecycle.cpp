#include "nfs/audio/speech/speech_queue.hpp"
#include "nfs/audio/speech/speech_rules.hpp"
#include "nfs/audio/speech/speech_interpreter.hpp"

namespace nfs::speech {

void reset_voice_selection() {
    for (auto &selection : state<VoiceSelection[8]>(8769728)) {
        selection.event = -1;
        selection.selected = 0;
    }
}


int scale_speech_parameter(int value, int scale, int mode) {
    const int product = static_cast<int>(static_cast<Dword>(value) * static_cast<Dword>(scale));
    const int scaled = product / 8;
    if (mode == 1)
        return scaled / 10;
    if (mode == 2)
        return static_cast<int>(static_cast<Dword>(scaled) * 2) / 7;
    return scaled;
}


Dword initialize(Dword resource_callback, Dword random_seed, Dword resource_argument) {
    clear_resource_callbacks();
    clear_callbacks();
    state<Dword>(8767936) = resource_callback;
    state<Dword>(8767908) = random_seed;
    state<Dword>(8767916) = resource_argument;
    state<decltype(&queue_event_callback)>(8739164) = &queue_event_callback;
    initialize_speech_random(random_seed);
    reset_utterance();
    set_speech_client_data(0);
    for (auto &value : state<Dword[8]>(8767968))
        value = 0;
    clear_event_groups();
    reset_voice_selection();
    clear_phrase_table_binding();
    reset_event_queue();
    state<Dword>(8739152) = 24681012;
    return 1;
}


void clear_callbacks() {
    state<Dword>(8767920) = 0;
    state<Dword>(8767924) = 0;
    state<Dword>(8767928) = 0;
}


void clear_resource_callbacks() {
    state<Dword>(8767936) = 0;
    state<Dword>(8767940) = 0;
    state<Dword>(8767944) = 0;
}


void shutdown() {
    if (state<Dword>(8739152) == 24681012) {
        state<Dword>(8739152) = 0;
        clear_resource_callbacks();
        clear_phrase_table_binding_on_shutdown();
        clear_event_groups();
    }
}


void clear_phrase_table_binding() {
    state<Dword>(8739156) = 0;
    state<Dword>(8739160) = 0;
}


void clear_phrase_table_binding_on_shutdown() {
    state<Dword>(8739156) = 0;
    state<Dword>(8739160) = 0;
}
} // namespace nfs::speech
