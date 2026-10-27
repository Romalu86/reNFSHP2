#include "nfs/audio/speech/speech_rules.hpp"
#include "nfs/audio/speech/speech_interpreter.hpp"
#include "nfs/audio/speech/speech_queue.hpp"

namespace nfs::speech {
namespace {
struct PhraseSelection {
    std::int16_t table;
    std::int16_t repetition;
    std::uint16_t phrase;
    std::uint16_t reserved;
};
static_assert(sizeof(PhraseSelection) == 8);
struct UtteranceRule {
    const Byte *rule;
    const Byte *sequence;
    Byte variant;
    Byte reserved;
    Byte state;
    Byte flags;
    Dword arguments[20];
};
static_assert(sizeof(UtteranceRule) == 92);
PhraseSelection &selection(int index) {
    return state<PhraseSelection>(8739188 + index * 8);
}
UtteranceRule &selected_rule(int index) {
    return state<UtteranceRule>(8739572 + index * 92);
}
} // namespace


void reset_utterance() {
    state<int>(8739180) = 0;
    state<int>(8739184) = 0;
    state<int>(8739176) = -1;
    for (int index = 0; index < 8; ++index) {
        auto &rule = selected_rule(index);
        rule.rule = nullptr;
        rule.sequence = nullptr;
        rule.variant = 0;
        rule.reserved = 0;
        rule.flags = 0;
    }
    for (int index = 0; index < 48; ++index) {
        auto &phrase = selection(index);
        phrase.table = -1;
        phrase.repetition = -1;
        phrase.phrase = 0;
        phrase.reserved = 0;
    }
}


Dword utterance_matches(int request_id) {
    return state<int>(8739176) == request_id && state<int>(8739180) > 0 ? 1 : 0;
}


int assemble_utterance(int request_id) {
    int phrase_count = 0;
    if (utterance_matches(request_id)) {
        const int count = state<int>(8739180);
        for (int index = 0; index < count; ++index) {
            auto &rule = selected_rule(index);
            Dword request = rule.arguments[0];
            notify_selected_rule(&request, rule.rule, rule.variant, rule.arguments);
            notify_phrase_bindings(&request, rule.rule, rule.sequence, phrase_count);
            emit_phrase_sequence(rule.sequence, &request, phrase_count);
            phrase_count += rule.sequence[4] >> 2;
            if (index == 0) {
                if (static_cast<std::int16_t>(request) ==
                    state<std::int16_t>(8769728 + request_id * 8)) {
                    Dword &repetitions = state<Dword>(8769732 + request_id * 8);
                    ++repetitions;
                } else {
                    state<Dword>(8769728 + request_id * 8) = request;
                    state<Dword>(8769732 + request_id * 8) = 1;
                }
            }
        }
    }
    reset_utterance();
    return phrase_count;
}


void notify_phrase_bindings(const Dword *request, const Byte *rule, const Byte *sequence,
                            int first_phrase) {
    if (state<BindingCallback>(8767944)) {
        Dword context;
        read_event_context(request, &context);
        const int count = sequence[4] >> 2;
        for (int index = 0; index < count; ++index) {
            const Byte *bindings = sequence + sequence[index + 8] * 4 + 12;
            for (int channel = 0; channel < 4; ++channel) {
                if (bindings[channel] != 255) {
                    const Dword binding = rule_binding(rule, bindings[channel]);
                    const auto &phrase = selection(first_phrase + index);
                    PhraseEntry entry;
                    if (read_phrase_entry(state<Byte **>(8739156)[phrase.table], phrase.phrase,
                                          &entry)) {
                        state<BindingCallback>(8767944)(
                            request, binding, 1u << (entry.values[channel] & 31), context);
                    }
                }
            }
        }
    }
}


int emit_phrase_sequence(const Byte *sequence, const Dword *request, int first_phrase) {
    if (!state<PhraseCallback>(8767936))
        return 0;
    int total_samples = 0;
    int voice = 0;
    Dword context;
    read_event_context(request, &context);
    read_event_voice(request, &voice);
    const int count = sequence[4] >> 2;
    for (int index = 0; index < count; ++index) {
        const auto &phrase = selection(first_phrase + index);
        const int table_index = phrase.table;
        const Dword phrase_index = phrase.phrase;
        Byte *table = state<Byte **>(8739156)[table_index];
        if ((table[4] & 128) && state<Dword>(7146808))
            consume_partition_phrase(table, static_cast<int>(phrase_index));
        PhraseEntry entry;
        if (read_phrase_entry(table, static_cast<int>(phrase_index), &entry)) {
            Dword offset = entry.start_sample;
            if (phrase.repetition != -1) {
                const Dword table_samples = *reinterpret_cast<const std::uint16_t *>(table + 8);
                offset +=
                    (table[7] + 1) * table_samples * static_cast<Dword>(phrase.repetition) * 256u;
            }
            total_samples = static_cast<int>(static_cast<Dword>(total_samples) + entry.samples);
            const PhrasePlayback playback = {table_index,
                                             static_cast<int>(offset),
                                             entry.samples,
                                             *request,
                                             static_cast<Dword>(voice),
                                             *reinterpret_cast<const std::uint16_t *>(table + 2),
                                             context};
            state<PhraseCallback>(8767936)(&playback);
            remember_table_phrase(table, static_cast<Byte>(phrase_index));
        }
    }
    return samples_to_centiseconds(total_samples);
}
} // namespace nfs::speech
