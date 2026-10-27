#include "nfs/audio/speech/speech_rules.hpp"

namespace nfs::speech {
namespace {
Byte **&tables() {
    return state<Byte **>(8739156);
}
int &capacity() {
    return state<int>(8739160);
}
template <class T> const T &field(const Byte *data, int offset) {
    return *reinterpret_cast<const T *>(data + offset);
}
int partition_offset(const Byte *table) {
    return 12 + ((table[4] & 15) + 2) * table[5] + ((table[4] >> 4) & 7) * 2;
}
int history_offset(const Byte *table) {
    return partition_offset(table) + ((table[4] & 128) ? ((table[5] + 7) / 8 + 1) : 0);
}
} // namespace


Dword phrase_table_storage_size(Dword count) {
    return count << 2;
}


void initialize_phrase_tables(int count, Byte **storage) {
    capacity() = count;
    tables() = storage;
    if (storage && count > 0)
        for (int index = 0; index < count; ++index)
            storage[index] = nullptr;
}


Byte *phrase_table(int index) {
    return state<Dword>(8739152) == 24681012 && tables() ? tables()[index] : nullptr;
}


std::int16_t find_phrase_table(std::int16_t id) {
    if (tables() && capacity() > 0) {
        std::int16_t index = 0;
        do {
            const Byte *table = tables()[index];
            if (table && field<std::int16_t>(table, 0) == id)
                return index;
            index = static_cast<std::int16_t>(static_cast<unsigned>(index) + 1);
        } while (index < capacity());
    }
    return -1;
}


std::int16_t find_phrase_table_variant(std::int16_t id, std::int16_t variant) {
    if (tables() && capacity() > 0) {
        std::int16_t index = 0;
        do {
            const Byte *table = tables()[index];
            if (table && field<std::int16_t>(table, 0) == id &&
                field<std::int16_t>(table, 2) == variant)
                return index;
            index = static_cast<std::int16_t>(static_cast<unsigned>(index) + 1);
        } while (index < capacity());
    }
    return -1;
}


Dword phrase_index_valid(int table_index, Dword phrase_index) {
    if (tables() && table_index >= 0) {
        const auto count = field<std::uint16_t>(tables()[table_index], 10);
        if (count != 65535 && phrase_index < count)
            return 1;
    }
    return 0;
}


void detach_phrase_table(int index) {
    if (tables())
        tables()[index] = nullptr;
}


void remember_table_phrase(Byte *table, Byte index) {
    if (table[6]) {
        Byte *history = table + history_offset(table);
        Byte cursor = *history;
        if (cursor < table[6])
            history[cursor + 1] = index;
        cursor = static_cast<Byte>(cursor + 1);
        if (cursor >= table[6])
            cursor = 0;
        *history = cursor;
    }
}


int phrase_history_distance(const Byte *table, Dword index) {
    const int count = table[6];
    if (count) {
        const Byte *history = table + history_offset(table);
        int cursor = *history;
        for (int age = 0; age < count; ++age) {
            if (--cursor < 0)
                cursor = count - 1;
            if (history[cursor + 1] == index)
                return age + 1;
        }
    }
    return 0;
}


void initialize_phrase_partition(Byte *table) {
    Byte *partition = table + partition_offset(table);
    const Dword count = *partition;
    if (count) {
        const Dword group = state<Dword>(8767908) % count;
        const Dword first = table[5] * group / count;
        const int length = static_cast<int>((group + 1) * table[5] / count - first);
        Byte *mask = partition + 1 + (first >> 3);
        Dword bit = first & 7;
        for (int index = 0; index < length; ++index) {
            *mask |= static_cast<Byte>(1u << bit);
            if (++bit == 8) {
                bit = 0;
                ++mask;
            }
        }
    }
}


int register_phrase_table(Byte *table) {
    if (!tables())
        return -1;
    if (table[6]) {
        const Byte *history = table + history_offset(table);
        if (*history)
            return -1;
        for (int index = 0; index < table[6]; ++index)
            if (history[index + 1] != 255)
                return -1;
    }
    const int index = vacant_phrase_table();
    if (index >= 0) {
        if (table[4] & 128)
            initialize_phrase_partition(table);
        tables()[index] = table;
    }
    return index;
}


int vacant_phrase_table() {
    for (int index = 0; index < capacity(); ++index)
        if (!tables()[index])
            return index;
    return -1;
}


Dword register_speech_rule(const Byte *rule, Dword bank_index) {
    return bank_index < 8 ? register_speech_rule_unchecked(rule, bank_index) : 0;
}


Dword register_speech_rule_unchecked(const Byte *rule, Dword bank_index) {
    if (rule[0] != 3 || rule[1] != 11)
        return 0;
    Dword id, variant;
    read_rule_identity(rule, &id, &variant);
    for (int index = 0; index < 8; ++index) {
        const Byte *registered = state<const Byte *>(8769664 + index * 8);
        if (registered && registered[9] == variant && registered[8] == id)
            return 0;
    }
    for (int index = 0; index < 8; ++index)
        if (!state<const Byte *>(8769664 + index * 8)) {
            state<const Byte *>(8769664 + index * 8) = rule;
            state<Dword>(8769668 + index * 8) = bank_index;
            return 1;
        }
    return 0;
}


void read_rule_identity(const Byte *rule, Dword *id, Dword *variant) {
    *id = rule[8];
    *variant = rule[9];
}


int rule_bit_value(Dword encoded) {
    const Dword value = encoded & 255;
    return static_cast<int>(state<Dword>(7146868 + (value >> 5) * 4) * (value & 31));
}


void consume_partition_phrase(Byte *table, int index) {
    Byte *mask = table + partition_offset(table);
    const int byte_index = index / 8 + 1;
    const int bit = index % 8;
    // SHL DL,CL keeps only eight result bits and masks the shift count to five bits.
    const Byte flag = static_cast<Byte>(1u << (static_cast<Dword>(bit) & 31));
    mask[byte_index] &= static_cast<Byte>(~flag);
}
} // namespace nfs::speech
