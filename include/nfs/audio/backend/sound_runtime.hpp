#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::sound {
using kernel::Dword;
using Byte = std::uint8_t;
using Callback = void(__cdecl *)();
struct Voice {
    alignas(4) Byte bytes[128];
};
struct BackendVoice {
    alignas(4) Byte bytes[260];
};
struct MixVoice {
    alignas(4) Byte bytes[80];
};
struct Bank {
    Dword *data;
    Byte loading, padding[3];
};
struct Heap {
    Byte *base;
    Dword *entries;
    Dword bytes, available, low_water;
    int count;
};
static_assert(sizeof(Heap) == 24);
struct Control {
    Byte initialized;
    std::int8_t master_volume;
    Byte unknown_2, lock_depth;
    std::int8_t producer_count, pump_count, filter_count;
    Byte unknown_7;
    std::int16_t voice_count;
    std::uint16_t voice_count_padding;
    Dword unknown_c;
    Callback producers[6], pumps[6], filters[4], shutdown[7];
    Voice *voices;
    Bank *banks;
};
static_assert(sizeof(Voice) == 128 && sizeof(BackendVoice) == 260 && sizeof(MixVoice) == 80);
static_assert(offsetof(Control, producers) == 16 && offsetof(Control, pumps) == 40 &&
              offsetof(Control, shutdown) == 80);
static_assert(offsetof(Control, voice_count) == 8 && offsetof(Control, voice_count_padding) == 10);
static_assert(offsetof(Control, voices) == 108 && offsetof(Control, banks) == 112 &&
              sizeof(Control) == 116);
extern Control control;
extern HANDLE mutex;
extern BackendVoice *&backends;
extern MixVoice *&mix_voices;
extern Dword &output_channels;
extern Byte effect_groups[84];
                                 // logical table extent.
extern const std::int16_t
    volume_lookup_storage[256];
extern const Byte pitch_curve[256];
template <class T, class O> T &field(O *object, unsigned offset) {
    return *reinterpret_cast<T *>(reinterpret_cast<Byte *>(object) + offset);
}
template <class T, class O> const T &field(const O *object, unsigned offset) {
    return *reinterpret_cast<const T *>(reinterpret_cast<const Byte *>(object) + offset);
}
template <class R, class... A> R backend_call(BackendVoice *backend, unsigned offset, A... args) {
    using Call = R(__cdecl *)(A...);
    return reinterpret_cast<Call>(field<Dword>(backend, offset))(args...);
}
void create_mutex();
void close_mutex();
void wait_mutex();
void release_mutex();
void lock();
void unlock();
int initialized();
int find_voice(Dword);
bool next_voice(int, int *);
void calculate_volume(int);
void submit_volume(int);
Dword set_master_volume(Byte);
void set_mix_volume(int, int, float);
int pitch_multiplier(int);
void calculate_pitch(int);
int submit_pitch(int);
void submit_pan(int);
int submit_effect(int, int);
void calculate_effect(int, int);
int set_voice_volume(Dword, int);
int set_voice_rate(Dword, Dword);
int set_voice_pan(Dword, std::int16_t, int);
int set_voice_effect(Dword, int, Byte);
int fade_voice_volume(Dword, int, int);
int set_voice_pitch(Dword, int);
int voice_volume(Dword);
int voice_missing(Dword);
int voice_time_remaining(Dword);
void add_producer(Callback);
void remove_producer(Callback);
void add_pump(Callback);
void pump();
void remove_pump(Callback);
void retire_voice(int);
int stop_voice(int);
int stop(Dword);
void stop_all();
void clear_backend_chain(void *);
void zero_memory(void *, Dword);
void initialize_heap(void *, Dword);
void limit_heap_gap(int *, int *);
void *allocate(Dword);
void release(void *);
int heap_used_percent();
int voice_position(int);
void *effect_group(int, Dword);
Dword effect_mask(Byte);
} // namespace nfs::sound
