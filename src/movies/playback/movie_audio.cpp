#include "movie_runtime.hpp"
#include <new>
#include <stdexcept>
namespace nfs::movies {

Audio *construct_audio(Audio *audio, files::streaming::Channel *channel, Dword request) {
    audio->request = -1;
    audio->buffer = nullptr;
    audio->stream = -1;
    if (!sound::initialized())
        throw std::runtime_error("MAD audio backend is unavailable");
    sound::VoiceParameters parameters;
    sound::initialize_parameters(&parameters);
    int bytes = sound::streaming::owned_bytes(1, 30);
    audio->buffer = memory.allocate("AV::audiobuff", bytes, 0, 0, memory.pool);
    if (!audio->buffer)
        throw std::bad_alloc();
    try {
    audio->stream =
        sound::streaming::create_channel_stream(channel, &parameters, 1, 30, audio->buffer, bytes);
    if (audio->stream < 0)
        throw std::runtime_error("MAD audio stream creation failed");
    audio->request = int(sound::streaming::enqueue_request(Dword(audio->stream), -1, request));
    if (audio->request < 0)
        throw std::runtime_error("MAD audio request could not be queued");
    } catch (...) {
        destroy_audio(audio);
        throw;
    }
    audio->gain = 1.0f;
    return audio;
}

void destroy_audio(Audio *audio) {
    if (audio->buffer) {
        if (audio->stream >= 0)
            sound::streaming::destroy_stream(Dword(audio->stream));
        kernel::memory_free(audio->buffer);
        audio->buffer = nullptr;
    }
    audio->stream = audio->request = -1;
}

int audio_rate(Audio *audio, Dword rate) {
    if (audio->stream == -1)
        return 0;
    sound::lock();
    int result = sound::streaming::set_stream_rate(Dword(audio->stream), rate);
    sound::unlock();
    return result;
}

int start_audio(Audio *audio) {
    if (audio->request == -1)
        return 0;
    sound::lock();
    int result = sound::streaming::set_request_delay(Dword(audio->request), 0);
    sound::unlock();
    return result;
}
} // namespace nfs::movies
