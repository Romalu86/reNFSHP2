#pragma once
#include "nfs/audio/backend/sound_runtime.hpp"
#include <initializer_list>
namespace nfs::game::runtime::race_audio::pending {
extern std::uint8_t &backend_stream_slot_count;
}
namespace nfs::sound::backend {
template <class R, class... A> R device_call(void *object, unsigned offset, A... args) {
    using Call = R(__stdcall *)(void *, A...);
    return reinterpret_cast<Call>(field<Dword *>(object, 0)[offset / 4])(object, args...);
}
extern Dword &stream_limits;
extern Dword *&stream_bindings;
extern Dword *const stream_devices;
extern void *&direct_sound;
extern Byte &use_effect_interface;
extern std::uint16_t caps_cached;
extern std::int16_t minimum_frequency;
extern int maximum_frequency;
extern const Byte buffer3d_iid[16], effect_iid[16], effect_guid[16];
extern const float device_rate_scale;
void query_device_limits(void *);
void device_set_buffer_ms(void *, int);
void device_set_pan(void *, Dword, Dword);
void device_set_effect(void *, int);
void device_set_rate(void *, int);
Dword device_position(void *);
Dword device_stopped(void *);
int device_remaining(void *);
int device_refill(void *, int, void *, void *);
int device_create(void *, const void *);
int device_start(void *);
void device_destroy(void *);
void mixer_set_pan(void *, Dword, Dword);
void mixer_set_effect(void *, int);
void mixer_set_voice_value(void *, Dword);
void mixer_set_rate(void *, int);
void mixer_seek(void *, Dword);
void mixer_set_loop(void *, Dword);
int mixer_position(void *);
int mixer_create(void *, const void *);
int mixer_start(void *);
void mixer_stop(void *);
void insert_processor(void **, void *);
void remove_processor(void **, void *);
void *find_stream(int);
void chain_set_value(void *, Dword);
void chain_set_loop(void *, Dword);
void chain_set_effect(void *, Dword);
int chain_remaining(void *);
int construct_buffer_chain(void *, const void *);
void start_mix_voice(int, int, int, Dword, Dword, int, Dword, Dword, Dword, int, Dword, Dword, Byte);
void activate_mix_voice(int);
void destroy_mix_voice(int);
void *construct_codec(Dword, int, Dword, int, Dword, Dword, int, Dword *);
void construct_converter(void *);
int construct_resampler(void *, const void *, int);
int root_voice(int);
int stream_slot(int);
void fill_pan(Dword, void *);
void set_mix_effect(int, int, float);
void voice_pan_vector(Dword, Dword, float *);
namespace pending {
extern Dword (&codec_constructors)[33], (&codec_sizes)[33];
extern Byte *const mix_format;
extern Byte &mix_modes;
using game::runtime::race_audio::pending::backend_stream_slot_count;
extern Dword *stream_nodes[32];
extern std::int8_t pan_table[1024];
int process_float(void *, int, void *, void *, Byte);
int process_resampler(void *, int, void *, void *, Byte);
void set_processor_rate(int, Dword);
void set_processor_seek(int, Dword);
void set_processor_loop(int, float);
void set_processor_effect(int, Dword);
int processor_position(int);
int create_filter(void *);
void update_filter(void *, const int *);
int create_effect(void *);
void change_effect(void *, const int *);
void seek_resampler(void *, Dword);
int sine_table(int);
int stream_samples(int);
} // namespace pending
} // namespace nfs::sound::backend
namespace nfs::sound::pending {
int start_backend(int, Dword, Dword, Dword, Dword, Dword, Dword, const Dword *, Dword,
                           const Dword *, Dword);
}
