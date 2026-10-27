#pragma once
#include "nfs/audio/speech/speech.hpp"
#include <cstdint>

namespace nfs::speech {
struct PhrasePlayback {
    int table_index;
    int sample_offset;
    int sample_length;
    Dword request_id;
    Dword voice_id;
    Dword bank_id;
    Dword context;
};
static_assert(sizeof(PhrasePlayback) == 28);
using PhraseCallback = void(__cdecl *)(const PhrasePlayback *);
using BindingCallback = void(__cdecl *)(const Dword *, Dword, Dword, Dword);
using TickCallback = Dword(__cdecl *)();

Dword phrase_table_storage_size(Dword capacity);
void initialize_phrase_tables(int capacity, Byte **tables);
Byte *phrase_table(int index);
std::int16_t find_phrase_table(std::int16_t id);
std::int16_t find_phrase_table_variant(std::int16_t id, std::int16_t variant);
Dword phrase_index_valid(int table_index, Dword phrase_index);
void detach_phrase_table(int index);
void remember_table_phrase(Byte *table, Byte index);
int phrase_history_distance(const Byte *table, Dword index);
void initialize_phrase_partition(Byte *table);
int register_phrase_table(Byte *table);
int vacant_phrase_table();
Dword register_speech_rule(const Byte *rule, Dword bank_index);
Dword register_speech_rule_unchecked(const Byte *rule, Dword bank_index);
Byte *speech_configuration();
Dword current_speech_tick();
Dword next_speech_random();
void initialize_speech_random(Dword seed);
void seed_speech_random(Dword seed);
std::uint16_t choose_phrase_index(int count, Dword tag);
int recent_phrase_distance(std::int16_t phrase, int count, std::int16_t tag);
void remember_phrase(std::int16_t phrase, std::int16_t tag);
void read_rule_identity(const Byte *rule, Dword *id, Dword *variant);
int rule_bit_value(Dword encoded);
void reset_utterance();
Dword utterance_matches(int request_id);
int assemble_utterance(int request_id);
void notify_phrase_bindings(const Dword *request, const Byte *rule, const Byte *sequence,
                            int first_phrase);
int emit_phrase_sequence(const Byte *sequence, const Dword *request, int first_phrase);
void consume_partition_phrase(Byte *table, int index);
int samples_to_centiseconds(int samples);
} // namespace nfs::speech
