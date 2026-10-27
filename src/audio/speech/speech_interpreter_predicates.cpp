#include "nfs/audio/speech/speech_interpreter.hpp"
#include "nfs/audio/speech/speech_queue.hpp"
#include "nfs/audio/speech/speech_rules.hpp"
#include <cstring>

namespace nfs::speech {
namespace {
template <class T> const T &field(const Byte *bytes, int offset) {
    return *reinterpret_cast<const T *>(bytes + offset);
}
int aligned_variants(const Byte *rule) {
    return (rule[6] * 2 + 3) & ~3;
}
} // namespace


int request_presence_matches(Dword descriptor, int value, int index) {
    const Dword operation = (descriptor >> ((static_cast<Dword>(index) * 2) & 31)) & 3;
    if (operation == 1)
        return value != 0;
    if (operation == 2)
        return value != -1;
    return 0;
}


int variant_request_allows(const Byte *rule, const Byte *variant, const Dword *request) {
    const int encoded = static_cast<std::int8_t>(rule[7]);
    const int count = encoded >> 4;
    if (count == 0)
        return 1;
    const Byte *masks = variant + (((variant[4] >> 2) + 3) & ~3) + 8;
    const Byte *binding = rule + (((encoded & 15) + 1) * 3 & ~3) + aligned_variants(rule) + 13;
    for (int i = 0; i < count; ++i) {
        const Dword mask = field<Dword>(masks, i * 4);
        if (mask == 0 || (request[binding[i * 3]] & mask) != 0)
            return 1;
    }
    return 0;
}


int variant_probability_allows(const Byte *variant) {
    return choose_phrase_index(100, 4294967295u) < variant[1];
}


void set_speech_client_data(Dword data) {
    state<Dword>(8767932) = data;
}


Dword rule_binding(const Byte *rule, int binding) {
    if (binding < 0 || binding >= 8)
        return 4294967295u;
    return rule[aligned_variants(rule) + binding * 3 + 12];
}


void notify_selected_rule(const Dword *event, const Byte *rule, int variant_index,
                          const Dword *request) {
    if (!state<RuleNotification>(8767944))
        return;
    Dword context;
    read_event_context(event, &context);
    const Byte *variant = rule + field<std::uint16_t>(rule, 12 + variant_index * 2) * 4;
    const Byte *binding = rule + aligned_variants(rule) + 12;
    const int count = rule[7] & 15;
    for (int i = 0; i < count; ++i, binding += 3) {
        const unsigned argument = binding[1];
        if ((binding[2] == 0 || binding[2] == 3) && variant_uses_argument(variant, argument))
            state<RuleNotification>(8767944)(event, binding[0], request[argument], context);
    }
}


int variant_uses_argument(const Byte *variant, Dword argument) {
    const int count = variant[4] >> 2;
    int found = 0;
    for (int i = 0; i < count; ++i) {
        const Byte *phrase = variant + variant[8 + i] * 4;
        if (phrase[2] == argument)
            return 1;
        for (int feature = 0; feature < 4; ++feature)
            if (phrase[8 + feature] == argument) {
                found = 1;
                break;
            }
    }
    return found;
}


Byte evaluate_rule_predicates(const Dword *event, const Byte *rule, const Dword *request,
                              Byte *unavailable) {
    Byte accepted = 0;
    Byte missing = 0;
    Dword context;
    read_event_context(event, &context);
    const int count = rule[7] & 15;
    const Byte *first = rule + aligned_variants(rule) + 12;
    for (int argument = 1; argument <= 20; ++argument) {
        const Byte *binding = first;
        for (int i = 0; i < count; ++i, binding += 3) {
            const unsigned key = binding[1];
            Dword value;
            if (argument == 20) {
                if (key != 0)
                    continue;
                value = 0;
            } else {
                if (key != static_cast<unsigned>(argument))
                    continue;
                value = request[key];
            }
            const Byte mask = static_cast<Byte>(Dword{1} << ((7 - i) & 31));
            const auto callback = state<RulePredicate>(8767940);
            const int result = callback ? callback(event, binding[0], value, context) : -1;
            if (result < 0)
                missing |= mask;
            else if (result > 0)
                accepted |= mask;
        }
    }
    *unavailable = missing;
    return accepted;
}


int variant_predicates_match(Byte accepted, Byte unavailable, const Byte *variant) {
    const Byte mask = static_cast<Byte>(variant[2] & ~unavailable);
    return (mask & accepted) == (variant[3] & mask);
}


void initialize_phrase_entry(PhraseEntry *entry) {
    entry->samples = 0;
    entry->start_sample = -1;
    entry->value_count = 0;
    entry->values[0] = entry->values[1] = entry->values[2] = entry->values[3] = 255;
}


int read_phrase_entry(const Byte *table, int index, PhraseEntry *entry) {
    initialize_phrase_entry(entry);
    const int count = table[5];
    if (index >= count)
        return 0;
    const int value_count = table[4] & 15;
    entry->value_count = value_count;
    const int stride = value_count + 2;
    const Byte *row = table + stride * index + 12;
    if (value_count != 0)
        std::memcpy(entry->values, row + 2, value_count);
    if (row[0] & 128) {
        index = row[1];
        row = table + stride * index + 12;
    }
    const Dword scale = (table[7] + 1) * 256;
    const Dword start = ((row[0] << 8) + row[1]) * scale;
    entry->start_sample = static_cast<int>(start);
    do {
        row += stride;
        if (++index >= count) {
            entry->samples = static_cast<int>(field<std::uint16_t>(table, 8) * scale - start);
            return 1;
        }
    } while (row[0] & 128);
    entry->samples = static_cast<int>(((row[0] << 8) + row[1]) * scale - start);
    return 1;
}
} // namespace nfs::speech
