#pragma once
#include "nfs/audio/speech/speech.hpp"

namespace nfs::speech {
struct EventGroup {
    Byte *table;
    int voice;
};
struct EventRecord {
    Dword tick;
    std::uint16_t sequence;
    Byte active;
    Byte voice;
    std::uint16_t chained;
    std::int16_t next;
    Byte *definition;
    Dword arguments[20];
};
struct VoiceSelection {
    std::int16_t event;
    std::uint16_t reserved;
    Dword selected;
};
static_assert(sizeof(EventGroup) == 8 && sizeof(EventRecord) == 96);
static_assert(offsetof(EventRecord, definition) == 12 && offsetof(EventRecord, arguments) == 16);
static_assert(sizeof(VoiceSelection) == 8);

Byte *find_event_definition(const Dword *);
Byte *find_group_event(Byte *, Dword);
int find_event_group(const Dword *, EventGroup **);
int read_event_voice(const Dword *, int *);
int read_event_context(const Dword *, Dword *);
void clear_event_groups();
std::uint16_t read_event_threshold(const Dword *);
void reset_event_queue();
Dword enqueue_event(const Dword *);
int find_queue_slot(Dword priority, Dword voice, int chained);
int has_event_chain(int voice);
void __cdecl queue_event_callback(Dword event, int argument_count, ...);
void clear_queued_events();
int queued_event_count(int voice);
void remove_event_chain(int index);
void reset_voice_selection();
int scale_speech_parameter(int, int, int);
Dword initialize(Dword resource_callback, Dword random_seed, Dword resource_argument);
void clear_callbacks();
void clear_resource_callbacks();
void shutdown();
void clear_phrase_table_binding();
void clear_phrase_table_binding_on_shutdown();
} // namespace nfs::speech
