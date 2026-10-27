#pragma once
#include "nfs/audio/speech/speech.hpp"

namespace nfs::lifecycle::construction {
extern kernel::Dword speech_handles[10800];
}
namespace nfs::speech {
struct PhrasePlayback;
}
namespace nfs::speech::game {
struct Owner {
    Dword methods;
    void *directory;
    void *table_storage;
    void *stream_memory;
    int stream;
    int enabled;
    Dword phrase_flags;
    int request_kind;
    int language;
    Byte transient[60];
};
struct BankRecord {
    int table;
    char name[80];
    Byte *header;
    Dword reserved;
    Dword data_offset;
};
static_assert(sizeof(Owner) == 96 && offsetof(Owner, language) == 32);
static_assert(sizeof(BankRecord) == 96 && offsetof(BankRecord, header) == 84);
inline BankRecord *banks() {
    return reinterpret_cast<BankRecord *>(lifecycle::construction::speech_handles);
}
extern const char *languages[7];
extern Dword voice_masks[6], paired_voice_masks[6], dispatch_masks[4];

void select_language(void *);
void post_initialize(void *);
void shutdown_owner(void *);
void load_level(void *);
void unload_level(void *);
void enter_frontend(void *);
void leave_frontend(void *);
void reset_race(void *);
void set_engines_disabled(Byte);
Dword play_phrase(Owner *, int table, int offset, int samples);
void __cdecl emit_phrase(const PhrasePlayback *);
void load_bank(Owner *, const char *, int, const char *, const char *);
void load_bank_catalog(Owner *);
void load_resources(Owner *);
void dispatch_police_event(void *, Dword event, int voice, Byte, Byte, Byte,
                           int violation, int vehicle_mask, Dword speed, int units, Byte,
                           int location);
Dword speed_phrase_mask(int);
} // namespace nfs::speech::game
