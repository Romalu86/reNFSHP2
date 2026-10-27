#include "movie_runtime.hpp"
#include "movie_clock_x87.hpp"
#include <cstring>
#include <mmsystem.h>
#include <new>
#include <stdexcept>
namespace nfs::movies {
namespace {
__declspec(noinline) Dword frame_argument_bits(const float *frame) {
    Dword bits;
    std::memcpy(&bits, frame, sizeof bits);
    return bits;
}
}

static Dword sample_clock(Clock *clock, Dword now, bool stamp_first) {
    const Dword delta = now - clock->sampled;
    const std::uint64_t increment =
        clock->rate == 4096 ? std::uint64_t(delta) : (std::uint64_t(delta) * clock->rate) >> 12;
    const Dword low = clock->low + Dword(increment);
    const Dword high = clock->high + Dword(increment >> 32) + Dword(low < clock->low);
    if (stamp_first)
        clock->sampled = now;
    clock->low = low;
    clock->high = high;
    if (!stamp_first)
        clock->sampled = now;
    return low;
}

void *start(Player *player, int buffers, int offset) {
    namespace streams = files::streaming;
    streams::set_priorities(player->audio_channel, 150, 50);
    streams::set_priorities(player->video, 150, 50);
    Dword bytes;
    if (player->video_load) {
        const int result = files::async::take(player->video_load, &player->video_memory, &bytes);
        player->video_load = 0;
        if (result != 1 || !player->video_memory || bytes < sizeof(files::streaming::Chunk))
            throw std::runtime_error("MAD video file load failed");
        player->video_request =
            streams::enqueue_memory(player->video, player->video_memory, int(bytes), 0);
        if (!player->video_request)
            throw std::runtime_error("MAD loaded video could not be queued");
    }
    if (player->audio_load) {
        const int result = files::async::take(player->audio_load, &player->audio_memory, &bytes);
        player->audio_load = 0;
        if (result != 1 || !player->audio_memory || bytes < sizeof(files::streaming::Chunk))
            throw std::runtime_error("MAD audio file load failed");
        player->audio_request =
            streams::enqueue_memory(player->audio_channel, player->audio_memory, int(bytes), 0);
        if (!player->audio_request)
            throw std::runtime_error("MAD loaded audio could not be queued");
    }
    if (!player->separate_audio)
        player->audio_request = player->video_request;
    if (!player->mute) {
        auto *audio = static_cast<Audio *>(memory.allocate("", 20, 0, 0, memory.pool));
        if (!audio)
            throw std::bad_alloc();
        try {
            player->audio = construct_audio(audio, player->audio_channel, player->audio_request);
        } catch (...) {
            memory.release(audio);
            throw;
        }
        player->synchronize_audio = player->audio->stream >= 0;
    } else
        player->synchronize_audio = 0;
    player->delegate = nullptr;
    if (memory.allocate && memory.release) {
        Source source;
        player->self = player;
        construct_source(&source, &player->self, acquire_callback, release_callback, buffers);
        auto *delegate = static_cast<Delegate *>(memory.allocate("", 28, 0, 0, memory.pool));
        if (!delegate)
            throw std::bad_alloc();
        player->delegate = construct_delegate(delegate, &source);
    }
    void *frame = decode_frame(player->delegate, 0);
    if (!frame)
        throw std::runtime_error("MAD stream contains no decodable frame");
    auto *clock = static_cast<Clock *>(memory.allocate("", 24, 0, 0, memory.pool));
    if (!clock)
        throw std::bad_alloc();
    clock->low = clock->high = 0;
    clock->sampled = timeGetTime();
    clock->rate = 4096;
    player->clock = clock;
    Dword now = sample_clock(clock, timeGetTime(), true);
    player->offset = offset;
    player->start_time = now;
    player->audio_position = 0;
    player->correction = 0;
    if (player->audio && start_audio(player->audio) < 0)
        throw std::runtime_error("MAD audio request could not be started");
    set_rate(player, 4096);
    return frame;
}

Dword set_rate(Player *player, Dword rate) {
    if (rate > 16384)
        rate = 16384;
    if (player->audio && audio_rate(player->audio, rate) < 0)
        throw std::runtime_error("MAD audio playback rate could not be applied");
    // Each branch accumulates with the previous rate. The normal-rate branch stores the new rate
    // before low/high time words; the scaled branch stores it between them.
    Clock *clock = player->clock;
    Dword now = timeGetTime();
    Dword delta = now - clock->sampled;
    if (clock->rate == 4096) {
        clock->rate = rate;
        Dword previous = clock->low;
        clock->low = previous + delta;
        clock->high += Dword(clock->low < previous);
    } else {
        std::uint64_t increment = (std::uint64_t(delta) * clock->rate) >> 12;
        Dword previous = clock->low;
        clock->low = previous + Dword(increment);
        clock->rate = rate;
        clock->high += Dword(increment >> 32) + Dword(clock->low < previous);
    }
    clock->sampled = now;
    return 0;
}

void pause(Player *player) {
    set_rate(player, 0);
}

void resume(Player *player) {
    set_rate(player, 4096);
}

int elapsed(Player *player) {
    Dword now = sample_clock(player->clock, timeGetTime(), false);
    int elapsed = int(now - player->start_time);
    if (player->synchronize_audio) {
        Dword stream_status[3], block_status[4];
        sound::lock();
        sound::streaming::stream_status(Dword(player->audio->stream), stream_status);
        sound::streaming::block_status(stream_status[1], block_status);
        sound::unlock();
        int audio = int(block_status[1] - Dword(sound::streaming::output_latency()));
        if (audio > player->audio_position) {
            int previous = player->correction;
            player->audio_position = audio;
            int correction = int(Dword(previous) - Dword(previous / 8) - Dword(elapsed) + Dword(audio));
            player->correction = correction;
            int negative = int(0u - Dword(correction));
            int maximum = correction > negative ? correction : negative;
            if (maximum > 264) {
                player->correction = 0;
                player->start_time -= Dword(correction / 8);
                return int(now - player->start_time);
            }
        }
    }
    return elapsed;
}

Byte frame_due(Player *player) {
    return clock_native::frame_due(player);
}

void *seek(Player *player, float frame) {
    return clock_native::seek(player, frame_argument_bits(&frame));
}
} // namespace nfs::movies
