#pragma once
#include "nfs/audio/streams/audio_streams.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/resources/files/resource_stream.hpp"
#include "nfs/movies/streams/movie_stream_events.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::movies {
using kernel::Dword;
using Byte = std::uint8_t;
using kernel::numeric::Extended;
inline Extended number(float value) {
    return Extended(value, kernel::numeric::current_format());
}
inline Extended integer(std::int64_t value) {
    return Extended::integer(value, kernel::numeric::current_format());
}
template <class T> T &field(void *object, unsigned offset) {
    return *reinterpret_cast<T *>(static_cast<Byte *>(object) + offset);
}
template <class R, class... A> R invoke(void *object, unsigned slot, A... arguments) {
    using Method = R(__thiscall *)(void *, A...);
    return reinterpret_cast<Method>(field<Dword *>(object, 0)[slot])(object, arguments...);
}
struct Memory {
    Dword reserved;
    void *(__cdecl *allocate)(const char *, int, Dword, Dword, Dword);
    Byte(__cdecl *release)(void *);
    Dword pool;
};
struct Chunk {
    void *token, *data;
    Dword bytes;
};
struct Delegate;
struct Player;
using Acquire = void(__cdecl *)(Delegate *, Player **, Chunk **);
using Retire = void(__cdecl *)(Delegate *, Player **, Chunk *);
struct Source {
    Player **context;
    Acquire acquire;
    Retire retire;
    int buffers;
};
struct Delegate {
    const Dword *methods;
    Chunk *chunk;
    Source source;
    void *decoder;
};
struct Clock {
    Dword sampled, reserved_4, low, high, rate, reserved_14;
};
struct Audio {
    float gain;
    void *buffer;
    Dword reserved_8;
    int stream, request;
};
struct Player {
    Audio *audio;
    void *video_buffer, *audio_buffer;
    const void *video_source, *audio_source;
    Byte separate_audio, reserved_15[3];
    int source_kind, mute;
    void *video_memory;
    Dword video_load;
    void *audio_memory;
    Dword audio_load;
    files::streaming::Channel *video, *audio_channel;
    Dword video_request, audio_request;
    int offset;
    Dword frame;
    float target_frame;
    Dword start_time;
    int audio_position, synchronize_audio, correction;
    Clock *clock;
    Player *self;
    Delegate *delegate;
};
static_assert(sizeof(Memory) == 16 && sizeof(Chunk) == 12 && sizeof(Source) == 16 &&
              sizeof(Delegate) == 28);
static_assert(sizeof(Audio) == 20 && sizeof(Clock) == 24 && sizeof(Player) == 104);
static_assert(offsetof(Player, video) == 48 && offsetof(Player, frame) == 68 &&
              offsetof(Player, delegate) == 100);
extern Memory memory;
extern Byte stop_requested;
extern const Dword delegate_methods[1], image_methods[1], renderer_methods[6],
    renderer_base_methods[6];
void initialize_memory(Memory *);
Player *construct_player(Player *, const char *, int, int, int);
Player *construct_separate_player(Player *, const void *, int, const void *, int, int, int);
Player *construct_stream_player(Player *, const void *, Dword, int, const void *, Dword, int, int, int);
Player *construct_memory_player(Player *, const void *, Dword, int, const void *, Dword, int, int);
void construct_streams(Player *, const void *, Dword, int, const void *, Dword, int, int, int);
void destroy_player(Player *);
Delegate *construct_delegate(Delegate *, const Source *);
Source *construct_source(Source *, Player **, Acquire, Retire, int);
void destroy_delegate(Delegate *);
void *__fastcall delete_delegate(Delegate *, void *, unsigned);
int bind_decoder(Delegate *, void *, Chunk *);
void release_decoder(Delegate *);
Dword current_frame(Delegate *);
Extended frame_rate(Delegate *);
void *decode_frame(Delegate *, int);
void retire_frame(Delegate *, void *);
Chunk *next_chunk(Delegate *);
void retire_chunk(Delegate *, Chunk *);
Chunk *construct_chunk(Chunk *);
void acquire_chunk(Player *, Delegate *, Chunk **);
void release_chunk(Player *, Chunk *);
void __cdecl acquire_callback(Delegate *, Player **, Chunk **);
void __cdecl release_callback(Delegate *, Player **, Chunk *);
void *start(Player *, int, int);
void *seek(Player *, float);
Byte frame_due(Player *);
Dword set_rate(Player *, Dword);
void pause(Player *);
void resume(Player *);
int elapsed(Player *);
Audio *construct_audio(Audio *, files::streaming::Channel *, Dword);
void destroy_audio(Audio *);
int audio_rate(Audio *, Dword);
int start_audio(Audio *);
struct Image {
    const Dword *methods;
    Dword *shape;
    float x, y, z, scale_x, scale_y, width, height;
    graphics::Device *device;
    Dword *context, *immediate;
    int format, bytes_per_pixel, conversion;
};
struct Renderer {
    const Dword *methods;
    Delegate *delegate;
    int current, count;
    Image *image;
    Dword *texture;
};
static_assert(sizeof(Image) == 60 && sizeof(Renderer) == 24);
Renderer *create_renderer(Delegate *, void *, graphics::Device *, Dword *, Dword *, int);
Renderer *construct_renderer(Renderer *, Delegate *, void *, graphics::Device *, Dword *, Dword *,
                             int);
Image *construct_image(Image *, void *, graphics::Device *, Dword *, Dword *, int);
void destroy_image(Image *);
void *__fastcall delete_image(Image *, void *, unsigned);
void destroy_renderer(Renderer *);
void *__fastcall delete_renderer(Renderer *, void *, unsigned);
void *__fastcall delete_renderer_base(Renderer *, void *, unsigned);
void __fastcall position(Renderer *, void *, float, float, float);
void __fastcall scale(Renderer *, void *, float, float);
void __fastcall draw_frame(Renderer *, void *, void *);
void __fastcall redraw(Renderer *, void *);
void __fastcall release_presented_frame(Renderer *, void *);
void copy_frame(Image *, void *, Dword *);
// Explicitly authorized movie upload compatibility, distinct from native TAR replacement.
void upload_frame(Image *, Dword *);
void register_movie_texture(Dword *);
void destroy_movie_texture(Dword *);
void release_movie_device_textures();
bool movie_texture_needs_upload(Dword *);
void prepare_frame(Renderer *, void *);
void prepare_presented_frame(Renderer *);
void draw_image(Image *, Dword *);
Dword *allocate_image(int, int, int, Dword, const char *);
int binary_exponent(int);
float *assign_vector(float *, float, float, float, float);
Byte any_input();
HWND foreground_owner();
namespace pending {
void *create_mad_decoder();
Dword convert_yuv(const Byte *, const Byte *, const Byte *, Dword, Dword, int, Byte *, Dword,
                 Dword);
} // namespace pending
} // namespace nfs::movies
namespace nfs::sound::streaming {
Dword enqueue_request(Dword, int, Dword);
int create_channel_stream(files::streaming::Channel *, const VoiceParameters *, int, int, void *,
                          int);
int set_request_delay(Dword, std::int16_t);
int set_stream_rate(Dword, Dword);
int output_latency();
} // namespace nfs::sound::streaming
