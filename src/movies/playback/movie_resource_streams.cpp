#include "movie_runtime.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <atomic>
#include <new>
#include <stdexcept>
namespace nfs::movies {
namespace {
// Movie-only compatibility state. The MAD codec and AV memory callbacks are
// already single-owner; publish pool identities before any file request starts.
std::atomic<Player *> stream_owner{};
std::atomic<files::streaming::Pool *> video_pool{}, audio_pool{};
std::atomic<bool> open_failed{};
void retire_async_load(Dword &handle) {
    const Dword request = handle;
    handle = 0;
    files::async::cancel(request);
    // A cancelled, unreleased ASYNCFILE request owns its load buffer until
    // finish/recycle invalidates the token. Completion alone precedes recycle.
    while (files::async::poll(request) >= 0) {
        kernel::cooperate(0);
        kernel::thread_yield(1);
    }
}
}
void bind_stream_owner(Player *player) {
    if (stream_owner.load(std::memory_order_acquire))
        throw std::runtime_error("MAD player already owns the decoder streams");
    video_pool.store(player->video->pool, std::memory_order_relaxed);
    audio_pool.store(player->audio_channel->pool, std::memory_order_relaxed);
    open_failed.store(false, std::memory_order_relaxed);
    stream_owner.store(player, std::memory_order_release);
}
void unbind_stream_owner(Player *player) noexcept {
    Player *expected = player;
    if (stream_owner.compare_exchange_strong(expected, nullptr, std::memory_order_acq_rel)) {
        video_pool.store(nullptr, std::memory_order_relaxed);
        audio_pool.store(nullptr, std::memory_order_relaxed);
    }
}
bool stream_failed(Player *player) noexcept {
    return stream_owner.load(std::memory_order_acquire) == player &&
           open_failed.load(std::memory_order_acquire);
}
bool notify_stream_open_failed(files::streaming::Pool *pool, Dword,
                              int) noexcept {
    if (!stream_owner.load(std::memory_order_acquire) ||
        (video_pool.load(std::memory_order_relaxed) != pool &&
         audio_pool.load(std::memory_order_relaxed) != pool))
        return false;
    open_failed.store(true, std::memory_order_release);
    return true;
}

Player *construct_player(Player *player, const char *path, int buffer_bytes, int source_kind,
                         int mute) {
    player->self = nullptr;
    construct_streams(player, path, 0, buffer_bytes, nullptr, 0, 0, source_kind, mute);
    return player;
}
// Separate sources without caller-supplied byte lengths.
Player *construct_separate_player(Player *player, const void *video, int video_buffer_bytes,
                                  const void *audio, int audio_buffer_bytes, int source_kind,
                                  int mute) {
    player->self = nullptr;
    construct_streams(player, video, 0, video_buffer_bytes, audio, 0, audio_buffer_bytes,
                       source_kind, mute);
    return player;
}
// Complete generic stream-owner constructor.
Player *construct_stream_player(Player *player, const void *video, Dword video_bytes,
                                int video_buffer_bytes, const void *audio, Dword audio_bytes,
                                int audio_buffer_bytes, int source_kind, int mute) {
    player->self = nullptr;
    construct_streams(player, video, video_bytes, video_buffer_bytes, audio, audio_bytes,
                       audio_buffer_bytes, source_kind, mute);
    return player;
}
// The saved source-kind argument is always2.
Player *construct_memory_player(Player *player, const void *video, Dword video_bytes,
                                int video_buffer_bytes, const void *audio, Dword audio_bytes,
                                int audio_buffer_bytes, int mute) {
    player->self = nullptr;
    construct_streams(player, video, video_bytes, video_buffer_bytes, audio, audio_bytes,
                       audio_buffer_bytes, 2, mute);
    return player;
}

void construct_streams(Player *player, const void *video, Dword video_bytes,
                          int video_buffer_bytes, const void *audio, Dword audio_bytes,
                          int audio_buffer_bytes, int source_kind, int mute) {
    int suppress_audio = sound::initialized() ? mute : 1;
    kernel::memory_fill(player, 0, 104);
    player->source_kind = source_kind;
    player->mute = suppress_audio;
    player->video_memory = player->audio_memory = nullptr;
    player->video_load = player->audio_load = 0;
    player->frame = 0;
    player->clock = nullptr;
    player->delegate = nullptr;
    player->audio = nullptr;
    player->video_source = video;
    player->audio_source = audio;
    if (!memory.allocate || !memory.release)
        throw std::runtime_error("MAD memory callbacks are unavailable");
    namespace streams = files::streaming;
    try {
    if (!audio) {
        player->separate_audio = 0;
        player->video_buffer =
            memory.allocate("AV::VideoStreamBuffer", video_buffer_bytes, 0, 0, memory.pool);
        if (!player->video_buffer)
            throw std::bad_alloc();
        player->audio_buffer = nullptr;
        player->video = streams::initialize(2, 3, 2, player->video_buffer, video_buffer_bytes);
        if (!player->video)
            throw std::runtime_error("MAD video stream buffer cannot be initialized");
        player->audio_channel = streams::channel(player->video, 2);
        if (!player->audio_channel)
            throw std::runtime_error("MAD audio channel cannot be initialized");
        streams::set_priorities(player->audio_channel, 151, 51);
        streams::set_priorities(player->video, 151, 51);
        streams::set_rule(player->video, 1, 255, 77, 1);
        streams::set_rule(player->video, 3, 0, 0, -2);
        streams::set_rule(player->video, 2, 65535, 17235, player->mute ? -1 : 2);
        bind_stream_owner(player);
        if (player->source_kind == 1)
            player->video_load = files::async::load(static_cast<const char *>(video), memory.pool);
        else if (!player->source_kind)
            player->video_request =
                streams::enqueue_file(player->video, static_cast<const char *>(video), 0, 0);
        else if (player->source_kind == 2)
            player->video_request =
                streams::enqueue_memory(player->video, player->video_source, int(video_bytes), 0);
    } else {
        player->separate_audio = 1;
        player->video_buffer =
            memory.allocate("AV::VideoStreamBuffer", video_buffer_bytes, 0, 0, memory.pool);
        player->audio_buffer =
            memory.allocate("AV::AudioStreamBuffer", audio_buffer_bytes, 0, 0, memory.pool);
        if (!player->video_buffer || !player->audio_buffer)
            throw std::bad_alloc();
        player->video = streams::initialize(2, 2, 1, player->video_buffer, video_buffer_bytes);
        player->audio_channel =
            streams::initialize(2, 2, 1, player->audio_buffer, audio_buffer_bytes);
        if (!player->video || !player->audio_channel)
            throw std::runtime_error("MAD separate stream buffer cannot be initialized");
        streams::set_priorities(player->audio_channel, 151, 51);
        streams::set_priorities(player->video, 151, 51);
        streams::set_rule(player->video, 1, 255, 77, 1);
        streams::set_rule(player->video, 2, 0, 0, -2);
        streams::set_rule(player->audio_channel, 1, 65535, 17235, player->mute ? -1 : 1);
        streams::set_rule(player->audio_channel, 2, 0, 0, -2);
        bind_stream_owner(player);
        if (player->source_kind == 1) {
            player->audio_load = files::async::load(static_cast<const char *>(audio), memory.pool);
            player->video_load = files::async::load(static_cast<const char *>(video), memory.pool);
        } else if (!player->source_kind) {
            player->audio_request = streams::enqueue_file(player->audio_channel,
                                                          static_cast<const char *>(audio), 0, 0);
            player->video_request =
                streams::enqueue_file(player->video, static_cast<const char *>(video), 0, 0);
        } else if (player->source_kind == 2) {
            player->video_request =
                streams::enqueue_memory(player->video, player->video_source, int(video_bytes), 0);
            player->audio_request = streams::enqueue_memory(
                player->audio_channel, player->audio_source, int(audio_bytes), 0);
        }
    }
    if ((source_kind == 0 && (!player->video_request ||
                              (player->separate_audio && !player->audio_request))) ||
        (source_kind == 1 && (!player->video_load ||
                              (player->separate_audio && !player->audio_load))) ||
        (source_kind == 2 && (!player->video_request ||
                              (player->separate_audio && !player->audio_request))))
        throw std::runtime_error("MAD stream request could not be queued");
    } catch (...) {
        destroy_player(player);
        throw;
    }
}

void destroy_player(Player *player) {
    if (player->delegate) {
        invoke<void *>(player->delegate, 0, 1u);
        player->delegate = nullptr;
    }
    Audio *audio = player->audio;
    if (audio) {
        destroy_audio(audio);
        memory.release(audio);
        player->audio = nullptr;
    }
    if (player->clock) {
        memory.release(player->clock);
        player->clock = nullptr;
    }
    if (player->video_load) {
        retire_async_load(player->video_load);
    }
    if (player->audio_load) {
        retire_async_load(player->audio_load);
    }
    files::streaming::destroy(player->video);
    player->video = nullptr;
    if (player->separate_audio) {
        files::streaming::destroy(player->audio_channel);
        player->audio_channel = nullptr;
    }
    if (player->video_memory) {
        kernel::memory_free(player->video_memory);
        player->video_memory = nullptr;
    }
    if (player->audio_memory) {
        kernel::memory_free(player->audio_memory);
        player->audio_memory = nullptr;
    }
    if (player->video_buffer) {
        kernel::memory_free(player->video_buffer);
        player->video_buffer = nullptr;
    }
    if (player->audio_buffer) {
        kernel::memory_free(player->audio_buffer);
        player->audio_buffer = nullptr;
    }
    // Pool destruction has completed the outstanding callbacks before the
    // private owner identity disappears. No worker dereferences Player here.
    unbind_stream_owner(player);
}
} // namespace nfs::movies
