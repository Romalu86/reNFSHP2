#include "nfs/audio/speech/speech_interpreter.hpp"
#include "nfs/audio/speech/speech_rules.hpp"
#include <cstring>

namespace nfs::speech {
namespace {
template <class T> const T &field(const Byte *bytes, int offset) {
    return *reinterpret_cast<const T *>(bytes + offset);
}
const Byte *lookup_phrase_table(int index) {
    return state<const Byte **>(8739156)[index];
}
} // namespace


int select_rule_phrases(RuleSelection *selection, const Byte *variant, int mode, int threshold) {
    clear_phrase_feature();
    if (mode == 1) {
        shuffle_rule_candidates(variant, selection);
        return select_short_rule(variant, selection, threshold);
    }
    const int count = variant[4] >> 2;
    for (int i = 0; i < count; ++i) {
        auto *candidates = &selection->phrases[i];
        if (candidates->count == 0 ||
            !select_phrase_candidate(selection, variant + variant[8 + i] * 4, candidates))
            return 0;
    }
    return 1;
}


void clear_phrase_feature() {
    state<Dword>(8767768) = 4294967295u;
    state<Dword>(8767772) = 0;
}


void shuffle_rule_candidates(const Byte *variant, RuleSelection *selection) {
    for (int phrase = 0; phrase < (variant[4] >> 2); ++phrase) {
        auto &candidates = selection->phrases[phrase];
        Byte *values = selection->candidates + candidates.first;
        for (int i = 0; i < candidates.count / 2; ++i) {
            const auto other =
                choose_phrase_index(candidates.count, static_cast<Dword>(candidates.table));
            const Byte saved = values[i];
            values[i] = values[other];
            values[other] = saved;
        }
    }
}


int select_short_rule(const Byte *variant, RuleSelection *selection, int threshold) {
    for (int i = 0; i < (variant[4] >> 2); ++i)
        selection->phrases[i].selected = selection->phrases[i].first;
    do {
        if (selection_duration(variant, selection) < threshold)
            return 1;
    } while (!advance_rule_selection(variant, selection));
    return 0;
}


int selection_duration(const Byte *variant, RuleSelection *selection) {
    Dword samples = 0;
    for (int i = 0; i < (variant[4] >> 2); ++i) {
        const auto &candidates = selection->phrases[i];
        PhraseEntry entry;
        if (read_phrase_entry(lookup_phrase_table(candidates.table),
                              selection->candidates[candidates.selected], &entry))
            samples += entry.samples;
    }
    return samples_to_centiseconds(static_cast<int>(samples));
}


int advance_rule_selection(const Byte *variant, RuleSelection *selection) {
    int index = (variant[4] >> 2) - 1;
    auto *candidates = selection->phrases + index;
    for (;;) {
        const int end = candidates->count + candidates->first;
        candidates->selected = static_cast<Byte>(candidates->selected + 1);
        if (candidates->selected < end)
            return 0;
        candidates->selected = candidates->first;
        if (--index < 0)
            return 1;
        --candidates;
    }
}


int select_phrase_candidate(RuleSelection *selection, const Byte *phrase,
                            PhraseCandidates *candidates) {
    const int count = static_cast<std::int8_t>(phrase[4]);
    const Byte *table = lookup_phrase_table(candidates->table);
    int selected = 0;
    bool has_feature = false;
    for (int feature = 0; feature < count; ++feature) {
        if (selected != 0)
            return selected;
        const Byte argument = phrase[8 + feature];
        if (argument == 254) {
            const auto choice =
                choose_phrase_index(candidates->count, static_cast<Dword>(candidates->table));
            candidates->selected = static_cast<Byte>(choice + candidates->first);
            const Dword key = phrase_feature_key(candidates->table, feature);
            PhraseEntry entry;
            if (read_phrase_entry(table, selection->candidates[candidates->selected], &entry)) {
                const unsigned value = entry.values[feature];
                set_phrase_feature(key, value == 255 ? 0 : Dword{1} << (value & 31));
            }
            selected = 1;
            has_feature = true;
        } else if (argument == 255) {
            const Dword mask = read_phrase_feature(phrase_feature_key(candidates->table, feature));
            const int available = candidates->count > 100 ? 0 : candidates->count;
            const Byte first = candidates->first;
            shuffle_candidates(selection->candidates + first, available);
            for (int i = 0; i < available; ++i) {
                const Byte choice = static_cast<Byte>(first + i);
                PhraseEntry entry;
                if (read_phrase_entry(table, selection->candidates[choice], &entry)) {
                    const unsigned value = entry.values[feature];
                    const Dword value_mask = value == 255 ? 0 : Dword{1} << (value & 31);
                    if (mask & value_mask) {
                        candidates->selected = choice;
                        selected = 1;
                    }
                }
            }
            has_feature = true;
        }
    }
    if (selected != 0)
        return selected;
    if (has_feature)
        return 0;
    candidates->selected = static_cast<Byte>(
        choose_phrase_index(candidates->count, static_cast<Dword>(candidates->table)) +
        candidates->first);
    return 1;
}


void set_phrase_feature(Dword key, Dword mask) {
    state<Dword>(8767768) = key;
    state<Dword>(8767772) = mask;
}


Dword read_phrase_feature(Dword key) {
    return state<Dword>(8767768) == key ? state<Dword>(8767772) : 0;
}


Dword phrase_feature_key(int table_index, Dword feature) {
    const Byte *table = lookup_phrase_table(table_index);
    const int count = (table[4] >> 4) & 7;
    const Byte *mapping = table + ((table[4] & 15) + 2) * table[5] + 12;
    for (int i = 0; i < count; ++i) {
        const Dword encoded = (mapping[0] << 8) + mapping[1];
        if ((encoded >> 13) == feature)
            return encoded & 8191;
        mapping += 2;
    }
    return 4294967295u;
}


void shuffle_candidates(Byte *values, int count) {
    for (int i = 0; i < count / 2; ++i) {
        const auto other = choose_phrase_index(count, static_cast<Dword>(count));
        const Byte saved = values[i];
        values[i] = values[other];
        values[other] = saved;
    }
}


void clear_rule_selection(RuleSelection *selection) {
    selection->candidate_count = 0;
    selection->phrase_count = 0;
}


void append_selected_rule(RuleSelection *selection, int voice, const Byte *rule,
                          const Byte *variant, Byte variant_index, const Dword *request) {
    if (state<int>(8739176) >= 0 && state<int>(8739176) != voice)
        return;
    const int slot = state<int>(8739180);
    state<Dword>(8739180) = static_cast<Dword>(slot) + 1;
    if (slot >= 8)
        return;
    state<int>(8739176) = voice;
    const unsigned offset = slot * 92;
    state<const Byte *>(8739572 + offset) = rule;
    state<const Byte *>(8739576 + offset) = variant;
    state<Byte>(8739580 + offset) = variant_index;
    std::memcpy(&state<Dword>(8739584 + offset), request, 20 * sizeof(Dword));
    state<Byte>(8739581 + offset) = 1;
    const int count = variant[4] >> 2;
    const int first = state<int>(8739184);
    for (int i = 0; i < count; ++i) {
        const auto &candidates = selection->phrases[i];
        const unsigned target = 8739188 + (first + i) * 8;
        state<std::int16_t>(target) = candidates.table;
        state<std::int16_t>(target + 2) = candidates.key;
        state<std::uint16_t>(target + 4) = selection->candidates[candidates.selected];
    }
    state<Dword>(8739184) += count;
}
} // namespace nfs::speech
