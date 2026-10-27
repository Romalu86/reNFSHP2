#pragma once
#include "nfs/audio/speech/speech.hpp"

namespace nfs::speech {
struct PhraseEntry {
    int samples;
    int start_sample;
    Dword value_count;
    Byte values[4];
};
static_assert(sizeof(PhraseEntry) == 16);
struct PhraseCandidates {
    std::int16_t table;
    std::int16_t key;
    Byte first;
    Byte count;
    Byte selected;
    Byte reserved;
};
static_assert(sizeof(PhraseCandidates) == 8);
struct RuleSelection {
    PhraseCandidates phrases[12];
    Byte phrase_count;
    Byte candidate_count;
    Byte candidates[100];
    Byte reserved[2];
};
static_assert(sizeof(RuleSelection) == 200);
using RulePredicate = int(__cdecl *)(const Dword *, Dword, Dword, Dword);
using RuleNotification = void(__cdecl *)(const Dword *, Dword, Dword, Dword);
using RequestRetry = int(__cdecl *)(int, Dword *);

int evaluate_rule_request(Dword *request);
void order_rule_variants(const Byte *rule, Byte *order);
int rule_history_allows(const Byte *rule, int voice);
int gather_rule_candidates(RuleSelection *, const Byte *variant, const Dword *request, Byte accepted,
                           Byte unavailable, int mode);
int resolve_phrase_candidates(const Byte *phrase, const Dword *request, PhraseCandidates *);
int gather_phrase_candidates(RuleSelection *, PhraseCandidates *, const Byte *phrase,
                             const Dword *request);
int phrase_values_match(int table, const PhraseEntry *, const Byte *phrase, const Dword *request);
int phrase_entry_allowed(const PhraseCandidates *, const Byte *table, int index);
Byte packed_bit(const Byte *bits, int index);
int phrase_key_allows(const PhraseCandidates *, const Byte *table, int index);
int variant_mode_allows(const Byte *variant, int mode);
int select_rule_phrases(RuleSelection *, const Byte *variant, int mode, int threshold);
void clear_phrase_feature();
void shuffle_rule_candidates(const Byte *variant, RuleSelection *);
int select_short_rule(const Byte *variant, RuleSelection *, int threshold);
int selection_duration(const Byte *variant, RuleSelection *);
int advance_rule_selection(const Byte *variant, RuleSelection *);
int select_phrase_candidate(RuleSelection *, const Byte *phrase, PhraseCandidates *);
void set_phrase_feature(Dword key, Dword mask);
Dword read_phrase_feature(Dword key);
Dword phrase_feature_key(int table, Dword feature);
void shuffle_candidates(Byte *values, int count);
void clear_rule_selection(RuleSelection *);
void append_selected_rule(RuleSelection *, int voice, const Byte *rule, const Byte *variant,
                          Byte variant_index, const Dword *request);
int request_presence_matches(Dword descriptor, int value, int index);
int variant_request_allows(const Byte *rule, const Byte *variant, const Dword *request);
int variant_probability_allows(const Byte *variant);
int evaluate_queued_request(int slot);
void set_speech_client_data(Dword data);
Dword rule_binding(const Byte *rule, int binding);
void notify_selected_rule(const Dword *event, const Byte *rule, int variant, const Dword *request);
int variant_uses_argument(const Byte *variant, Dword argument);
Byte evaluate_rule_predicates(const Dword *event, const Byte *rule, const Dword *request,
                              Byte *unavailable);
int variant_predicates_match(Byte accepted, Byte unavailable, const Byte *variant);
void initialize_phrase_entry(PhraseEntry *);
int read_phrase_entry(const Byte *table, int index, PhraseEntry *);
} // namespace nfs::speech
