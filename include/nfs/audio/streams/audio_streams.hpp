#pragma once
#include "nfs/audio/backend/sound_ownership.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/resources/files/resource_stream.hpp"
namespace nfs::game::runtime::race_audio {
void *stream(nfs::sound::Dword);
}
namespace nfs::sound::streaming {
int stream_volume(Dword);
struct Format {
    std::uint16_t sample_rate;
    Byte channels, codec;
};
struct Parameters {
    std::uint16_t pitch;
    Byte group, volume, pan, effect, pitch_depth, flags;
    std::uint16_t mode, reserved_0a;
    std::int16_t channel_pan[4];
    void *codec_data[4];
    Dword codec_bytes[4];
    void *envelopes[4];
    Dword envelope_bytes[4];
};
struct Metadata {
    Dword samples, channel_offsets[4];
};
struct Block {
    Block *next, *previous;
    Dword request, handle, byte_rate, consumed, total, pending;
    int delay;
    Byte started, reserved_25[3];
};
struct BlockQueue {
    Block *first, *last;
    int count;
};
struct Stream {
    files::streaming::Channel *source;
    int voice, decoder;
    Dword generation;
    Byte state, borrowed_source, channel_levels_enabled, reserved_13;
    Byte channel_levels[16];
    Format active_format, pending_format;
    Parameters active, pending;
    VoiceParameters voice_parameters;
    Dword processor[6];
    BlockQueue playing, available;
    Block *current;
};
static_assert(sizeof(Format) == 4 && sizeof(Parameters) == 84 && sizeof(Block) == 40 &&
              sizeof(Stream) == 288);
static_assert(offsetof(Parameters, codec_data) == 20 && offsetof(Parameters, envelopes) == 52);
static_assert(offsetof(Stream, active_format) == 36 && offsetof(Stream, active) == 44 &&
              offsetof(Stream, pending) == 128 && offsetof(Stream, voice_parameters) == 212 &&
              offsetof(Stream, playing) == 260 && offsetof(Stream, available) == 272);
inline Stream *lookup(Dword handle) {
    return static_cast<Stream *>(game::runtime::race_audio::stream(handle));
}
void initialize_queue(BlockQueue *);
void push_front(BlockQueue *, Block *);
void push_back(BlockQueue *, Block *);
Block *pop_front(BlockQueue *);
void remove(BlockQueue *, Block *);
Block *find_block(Dword);
void retire_block(Dword);
void block_released(Dword, Dword);
void samples_consumed(Dword, Dword, Dword);
int byte_rate(const Format *);
int live_count();
int shutdown();
void pump_streams();
void start_decoder(Stream *);
int should_wait(Stream *);
void decode_header(Dword, const void *, Format *, Parameters *, Metadata *);
int initialize_stream_parameters(Parameters *);
int parse_header(int, const files::streaming::Chunk *);
void submit_data(Stream *, files::streaming::Chunk *);
int dispatch_chunk(int, files::streaming::Chunk *);
int create(const VoiceParameters *, int, int, void *, int, files::streaming::Channel *, int);
int create_file_stream(const VoiceParameters *, int, int, void *, int);
Dword enqueue(Dword, int, const void *, int, int);
Dword enqueue_file(Dword, int, const char *, int);
int stop_stream(Dword);
int destroy_stream(Dword);
int set_prebuffer(Dword, int);
int owned_bytes(int, int);
int file_stream_bytes(int, int);
int set_processor(Dword, const Dword *);
int clear_processor(Dword, Dword);
int clear_voice_processor(Dword, Dword);
int detach_voice_processor(int, Dword);
void remove_channel_processor(int, Dword);
int stream_status(Dword, Dword *);
int block_status(Dword, Dword *);
std::uint64_t multiply_wide(Dword, Dword);
Dword divide_wide(Dword, Dword, Dword);
int start_stream_voice(int, const Format *, const Parameters *, const VoiceParameters *);
int start_stream_backend(int, int, std::uint16_t, Dword, Dword, Dword, const Format *, void *const *);
int set_voice_processor(Dword, const Dword *);
int set_voice_channel_levels(Dword, const Byte *);
int attach_voice_processor(int, const Dword *);
void apply_voice_channel_levels(int, const Byte *);
void insert_channel_processor(int, int, const Dword *);
} // namespace nfs::sound::streaming
