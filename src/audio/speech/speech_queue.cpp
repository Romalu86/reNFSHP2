#include "nfs/audio/speech/speech_queue.hpp"
#include "nfs/audio/speech/speech_rules.hpp"
#include <cstdarg>
#include <cstring>

namespace nfs::speech {

void reset_event_queue() {
    for (unsigned voice = 0; voice < 8; ++voice) {
        state<Dword[8]>(8768000)[voice] = 0;
        state<int[8]>(8768032)[voice] = -1;
        state<int[8]>(8768064)[voice] = -1;
    }
    state<Dword>(8768096) = 0;
    for (auto &event : state<EventRecord[16]>(8768100)) {
        event.active = 0;
        event.sequence = 0;
        event.tick = 0;
        event.definition = nullptr;
        event.voice = 255;
        std::memset(event.arguments, 0, sizeof(event.arguments));
    }
    state<Dword>(8767912) = 0;
    state<std::uint16_t>(8767904) = 0;
}


Dword enqueue_event(const Dword *arguments) {
    Dword accepted = 0;
    if (state<Dword>(8739144) == 0) {
        state<Dword>(8739144) = 1;
        const Dword identifier = arguments[0];
        Byte *definition = find_event_definition(&identifier);
        int voice;
        if (definition && read_event_voice(&identifier, &voice)) {
            const int chained = (definition[10] >> 5) & 1;
            int root = -1;
            int previous = -1;
            Dword priority;
            bool eligible = true;
            if (chained) {
                eligible = has_event_chain(voice) != 0;
                if (eligible) {
                    root = state<int[8]>(8768064)[voice];
                    previous = state<int[8]>(8768032)[voice];
                    priority = *reinterpret_cast<const std::uint16_t *>(
                        state<EventRecord[16]>(8768100)[root].definition + 4);
                }
            } else {
                const int draw = static_cast<int>(choose_phrase_index(100, 4294967295u) & 65535);
                eligible = draw <= static_cast<std::int8_t>(definition[9]);
                priority = *reinterpret_cast<const std::uint16_t *>(definition + 4);
            }
            if (eligible) {
                const int index = find_queue_slot(priority, static_cast<Dword>(voice), chained);
                if (index < 0) {
                    if (chained)
                        remove_event_chain(root);
                } else {
                    const Dword tick = current_speech_tick();
                    auto &sequence = state<std::uint16_t>(8767904);
                    if (tick == state<Dword>(8767912))
                        sequence = static_cast<std::uint16_t>(sequence + 1);
                    else
                        sequence = 0;
                    state<Dword>(8767912) = tick;
                    auto &events = state<EventRecord[16]>(8768100);
                    auto &event = events[index];
                    event.definition = definition;
                    event.voice = static_cast<Byte>(voice);
                    event.sequence = sequence;
                    event.tick = tick;
                    event.chained = static_cast<std::uint16_t>(chained);
                    event.next = -1;
                    if (chained)
                        events[previous].next = static_cast<std::int16_t>(index);
                    std::memcpy(event.arguments, arguments, sizeof(event.arguments));
                    if (!chained)
                        ++state<Dword[8]>(8768000)[voice];
                    event.active = 1;
                    state<int[8]>(8768032)[voice] = index;
                    if (!chained)
                        state<int[8]>(8768064)[voice] = index;
                    accepted = 1;
                }
            }
        }
    }
    // The saved reentrant path also clears this flag.
    state<Dword>(8739144) = 0;
    return accepted;
}


int find_queue_slot(Dword priority, Dword voice, int chained) {
    auto &events = state<EventRecord[16]>(8768100);
    for (int index = 0; index < 16; ++index)
        if (!events[index].active)
            return index;
    const int retained = chained ? state<int[8]>(8768064)[voice] : -1;
    const Dword tick = current_speech_tick();
    for (int index = 0; index < 16; ++index) {
        const auto &event = events[index];
        if (!event.chained) {
            const Dword lifetime = *reinterpret_cast<const std::uint16_t *>(event.definition + 2);
            if (lifetime && tick - event.tick > lifetime && index != retained) {
                remove_event_chain(index);
                return index;
            }
        }
    }
    for (int index = 0; index < 16; ++index) {
        auto &event = events[index];
        if (!event.chained &&
            *reinterpret_cast<const std::uint16_t *>(event.definition + 4) <= priority &&
            event.voice == voice && index != retained) {
            event.active = 0;
            --state<Dword[8]>(8768000)[event.voice];
            return index;
        }
    }
    return -1;
}


int has_event_chain(int voice) {
    const int last = state<int[8]>(8768032)[voice];
    const int root = state<int[8]>(8768064)[voice];
    return last >= 0 && last < 16 && root >= 0 && root < 16;
}


void __cdecl queue_event_callback(Dword event, int argument_count, ...) {
    Dword arguments[20];
    arguments[0] = event;
    std::va_list values;
    va_start(values, argument_count);
    for (int index = 0; index < argument_count; ++index)
        arguments[index + 1] = va_arg(values, Dword);
    va_end(values);
    enqueue_event(arguments);
}


void clear_queued_events() {
    for (auto &event : state<EventRecord[16]>(8768100)) {
        if (event.active) {
            event.active = 0;
            --state<Dword[8]>(8768000)[event.voice];
        }
    }
    for (auto &count : state<Dword[8]>(8768000))
        if (count)
            count = 0;
    reset_voice_selection();
}


int queued_event_count(int voice) {
    return static_cast<int>(state<Dword[8]>(8768000)[voice]);
}


void remove_event_chain(int index) {
    if (index < 0 || index >= 16)
        return;
    auto &events = state<EventRecord[16]>(8768100);
    auto &event = events[index];
    if (event.chained)
        return;
    event.active = 0;
    const unsigned voice = event.voice;
    if (index == state<int[8]>(8768064)[voice])
        state<int[8]>(8768064)[voice] = -1;
    if (index == state<int[8]>(8768032)[voice])
        state<int[8]>(8768032)[voice] = -1;
    int next = event.next;
    while (next >= 0 && next < 16) {
        auto &linked = events[next];
        if (!linked.chained || !linked.active)
            break;
        if (next == state<int[8]>(8768032)[voice])
            state<int[8]>(8768032)[voice] = -1;
        linked.active = 0;
        next = linked.next;
    }
    if (!event.chained)
        --state<Dword[8]>(8768000)[voice];
    if (static_cast<int>(state<Dword[8]>(8768000)[voice]) < 0)
        state<Dword[8]>(8768000)[voice] = 0;
}
} // namespace nfs::speech
