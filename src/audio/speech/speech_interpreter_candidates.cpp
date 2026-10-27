#include "nfs/audio/speech/speech_interpreter.hpp"
#include "nfs/audio/speech/speech_rules.hpp"

namespace nfs::speech {
namespace {
template <class T> const T &field(const Byte *bytes, int offset) {
    return *reinterpret_cast<const T *>(bytes + offset);
}
const Byte *lookup_phrase_table(int index) {
    return state<const Byte **>(8739156)[index];
}
} // namespace


void order_rule_variants(const Byte *rule, Byte *order) {
    int weights[100];
    int total = 0;
    const int count = rule[6];
    for (int i = 0; i < count; ++i) {
        const Byte encoded = rule[field<std::uint16_t>(rule, 12 + i * 2) * 4];
        weights[i] = rule_bit_value(encoded);
        total = static_cast<int>(static_cast<Dword>(total) + rule_bit_value(encoded));
    }
    int output = 0;
    for (;;) {
        if (total < 1) {
            for (int i = 0; i < count; ++i)
                if (rule_bit_value(rule[field<std::uint16_t>(rule, 12 + i * 2) * 4]) == 0)
                    order[output++] = static_cast<Byte>(i);
            return;
        }
        Dword choice = choose_phrase_index(total, 4294967295u);
        int index = 0;
        while (index < count) {
            choice -= weights[index];
            if (static_cast<int>(choice) < 0)
                break;
            ++index;
        }
        order[output++] = static_cast<Byte>(index);
        total = static_cast<int>(static_cast<Dword>(total) - weights[index]);
        weights[index] = 0;
    }
}


int rule_history_allows(const Byte *rule, int voice) {
    return !(field<std::int16_t>(rule, 0) == state<std::int16_t>(8769728 + voice * 8) &&
             static_cast<std::int8_t>(rule[8]) > 0 &&
             static_cast<std::int8_t>(rule[8]) <= state<int>(8769732 + voice * 8));
}


int gather_rule_candidates(RuleSelection *selection, const Byte *variant, const Dword *request,
                           Byte accepted, Byte unavailable, int mode) {
    const unsigned count = variant[4] >> 2;
    if (count > 12 || !variant_mode_allows(variant, mode) ||
        !variant_predicates_match(accepted, unavailable, variant))
        return 0;
    selection->phrase_count = static_cast<Byte>(count);
    for (unsigned i = 0; i < count; ++i) {
        auto *candidates = &selection->phrases[i];
        candidates->first = selection->candidate_count;
        const Byte *phrase = variant + variant[8 + i] * 4;
        if (!resolve_phrase_candidates(phrase, request, candidates))
            return 0;
        const int choices = gather_phrase_candidates(selection, candidates, phrase, request);
        if (choices == 0)
            return 0;
        candidates->count = static_cast<Byte>(choices);
    }
    return 1;
}


int resolve_phrase_candidates(const Byte *phrase, const Dword *request,
                              PhraseCandidates *candidates) {
    candidates->key = -1;
    candidates->table = -1;
    const auto id = field<std::int16_t>(phrase, 0);
    const unsigned argument = phrase[2];
    switch (static_cast<std::int8_t>(phrase[3])) {
    case 0:
        candidates->table = find_phrase_table(id);
        break;
    case 1:
        if (argument <= 19)
            candidates->table =
                find_phrase_table_variant(id, static_cast<std::int16_t>(request[argument]));
        break;
    case 2: {
        const auto table = find_phrase_table(id);
        if (phrase_index_valid(table, request[argument])) {
            candidates->table = table;
            candidates->key = static_cast<std::int16_t>(request[argument]);
        }
        break;
    }
    }
    return candidates->table >= 0;
}


int gather_phrase_candidates(RuleSelection *selection, PhraseCandidates *candidates,
                             const Byte *phrase, const Dword *request) {
    const Byte *table = lookup_phrase_table(candidates->table);
    const int count = table[5];
    int available = 0;
    int best_distance = 0;
    int best_index = -1;
    int result = 0;
    for (int index = 0; index < count; ++index) {
        PhraseEntry entry;
        result = read_phrase_entry(table, index, &entry);
        if (!result)
            continue;
        result = phrase_values_match(candidates->table, &entry, phrase, request);
        if (!result)
            continue;
        result = phrase_entry_allowed(candidates, table, index);
        if (!result)
            continue;
        result = phrase_history_distance(table, index);
        if (result < 1) {
            if (selection->candidate_count < 100) {
                selection->candidates[selection->candidate_count++] = static_cast<Byte>(index);
                ++available;
            }
        } else if (best_distance < result) {
            best_distance = result;
            best_index = index;
        }
    }
    if (count != 0) {
        result = available;
        if (available == 0 && best_index != -1 && selection->candidate_count < 100) {
            selection->candidates[selection->candidate_count++] = static_cast<Byte>(best_index);
            return 1;
        }
    }
    return result;
}


int phrase_values_match(int, const PhraseEntry *entry, const Byte *phrase, const Dword *request) {
    const int count = static_cast<std::int8_t>(phrase[4]);
    if (count > 4)
        return 0;
    for (int i = 0; i < count; ++i) {
        const unsigned value = entry->values[i];
        if (value > 31)
            return 0;
        const Dword mask = Dword{1} << value;
        if ((field<Dword>(phrase, 16 + i * 4) & mask) == 0)
            return 0;
        const unsigned argument = phrase[8 + i];
        if (argument != 0 && argument != 255 && argument != 254 && (request[argument] & mask) == 0)
            return 0;
    }
    return 1;
}


int phrase_entry_allowed(const PhraseCandidates *candidates, const Byte *table, int index) {
    if (table[5] < index)
        return 0;
    int allowed = 1;
    if (candidates->key != -1) {
        allowed = phrase_key_allows(candidates, table, index);
        if (!allowed)
            return 0;
    }
    if (table[4] & 128) {
        const int offset = ((table[4] & 15) + 2) * table[5] + ((table[4] >> 4) & 7) * 2 + 13;
        return packed_bit(table + offset, index);
    }
    return allowed;
}


Byte packed_bit(const Byte *bits, int index) {
    const int remainder = index % 8;
    return static_cast<Byte>((Dword{1} << (static_cast<Dword>(remainder) & 31)) & bits[index / 8]);
}


int phrase_key_allows(const PhraseCandidates *candidates, const Byte *table, int index) {
    if (field<std::uint16_t>(table, 10) <= candidates->key)
        return 0;
    const int count = table[5];
    int offset = ((table[4] & 15) + 2) * count + 12 + ((table[4] >> 4) & 7) * 2;
    if (table[4] & 128)
        offset += ((count + 7) >> 3) + 1;
    if (table[6] != 0)
        offset += 1 + table[6];
    return packed_bit(table + offset, index + count * candidates->key);
}


int variant_mode_allows(const Byte *variant, int mode) {
    switch (variant[4] & 3) {
    case 0:
        return 1;
    case 1:
        return mode == 2;
    case 2:
        return mode != 2;
    default:
        return 0;
    }
}
} // namespace nfs::speech
