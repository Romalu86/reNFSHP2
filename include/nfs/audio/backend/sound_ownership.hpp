#pragma once
#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend {
// Storage for sound configuration, devices, mixing and deferred events.
extern Byte configuration_bytes[360];
extern Byte platform_bytes[88];
extern Byte lifecycle_bytes[56];
extern Byte mixer_bytes[516];
extern Byte platform_tail_bytes[36];
extern Byte sample_bindings[256 * 24];
extern int deferred_count;
extern Byte deferred_events[768];
template <class T> T &setting(unsigned va) {
    return field<T>(configuration_bytes, va - 8708856);
}
template <class T> T &platform(unsigned va) {
    return field<T>(platform_bytes, va - 8711368);
}
template <class T> T &lifecycle(unsigned va) {
    return field<T>(lifecycle_bytes, va - 8708800);
}
template <class T> T &mixer(unsigned va) {
    return field<T>(mixer_bytes, va - 8717696);
}
template <class T> T &platform_tail(unsigned va) {
    return field<T>(platform_tail_bytes, va - 8717636);
}
Dword query_cpu_features();
int find_voice_channel(int);
int stream_buffer_bytes(int);
int stream_total_bytes(int);
int largest_voice_channel(int,int);
int stream_platform_bytes();
int initialize_stream_platform(int, void *);
int release_stream_platform(int);
int create_stream_queue(Dword, Dword, Dword, void *, Dword);
int queue_stream_block(int, const void *);
int stream_free_blocks(int);
int stop_stream_queue(int);
int release_stream_queue(int);
int read_stream_block(int, int, int *, int *);
void *consume_stream_samples(int, int, int);
void dispatch_stream_events();
void *foreground_window();
int probe_device(Dword *, int *);
void probe_effect_interface(int);
void release_effect_interface();
int apply_environment(int, int, int);
int create_output_device();
int release_output_device();
int stop_output_device();
int initialize_backend();
int shutdown_backend();
void start_backend_thread();
void stop_backend_thread();
DWORD WINAPI backend_thread(void *);
void pump_backend();
void restore_output_buffers();
void render_output(void *, int);
void prime_output_ring();
void render_mixer(void *, int);
int find_sample_slot();
int rebuild_sample_buffer(int);
int create_sample_buffer(Byte, std::uint16_t, Dword, int, int, void *);
int release_sample_buffer(int);
void *bind_sample_owner(void *);
void unbind_sample_owner(void *);
void reset_mixer_filter(float);
void initialize_mixer(const void *);
void shutdown_mixer();
void bind_mixer_methods();
void build_pan_table();
int pan_curve(int);
void set_mixer_mode(Byte);
int load_configuration(void *);
int apply_configuration(const void *);
int validate_configuration();
int discover_configuration();
int initialize_audio(void *, Dword);
int shutdown_audio();
int register_audio_tasks();
void at_exit_shutdown();
int configure_effect_groups(Dword, Dword, int, Dword, Dword);
int set_effect_group_flags(Dword, Dword);
void update_effect_group(Byte);
int refresh_effect_group(Dword, Dword);
void *codec_allocate(Dword);
void codec_release(void *);
void register_codec_allocator(void *(__cdecl *)(Dword));
void register_codec_release(void(__cdecl *)(void *));
namespace pending {
using Processor = int(__cdecl *)(void *, int, void *, void *, Byte);
using Constructor = void(__cdecl *)(void *, void *);
extern void *(__cdecl *codec_allocator)(Dword);
extern void(__cdecl *codec_deallocator)(void *);
void register_pcm_codecs();
void register_pcm16_codecs();
void register_compressed_codecs();
void register_speech_codecs();
void build_pcm8_table();
extern float pcm8_table[256];
void construct_pcm(void *, void *);
void construct_loop_pcm(void *, void *);
void construct_stream_pcm(void *, void *);
void construct_speech(void *, void *);
void construct_stream_speech(void *, void *);
void construct_compressed(void *, void *);
void construct_stream_compressed(void *, void *);
void construct_loop_compressed(void *, void *);
void mix_scalar(int, float, const float *, float *);
void mix_packed(int, float, const float *, float *);
void mix_streaming(int, float, const float *, float *);
void pcm16_scalar(int, const std::int16_t *, float *);
void pcm16_packed(int, const std::int16_t *, float *);
void pcm8_scalar(int, const Byte *, float *);
void scale_samples(float, float, const float *, float *);
} // namespace pending
} // namespace nfs::sound::backend
