#include "movie_runtime.hpp"
#include <new>
#include <stdexcept>
namespace nfs::movies {

static Byte is_mad_chunk(Dword tag) {
    // Byte string: mDAMeDAMkDAM. The caller reverses the stream bytes first.
    for (Dword candidate : {1296122989u, 1296122981u, 1296122987u})
        if (tag == candidate)
            return 1;
    return 0;
}

void acquire_chunk(Player *player, Delegate *delegate, Chunk **output) {
    auto *stream = player->video;
    *output = nullptr;
    for (;;) {
        auto *chunk = files::streaming::take_chunk(stream);
        kernel::cooperate(0);
        if (stream_failed(player)) {
            if (chunk)
                files::streaming::release_chunk(stream, chunk);
            throw std::runtime_error("MAD stream open failed");
        }
        if (chunk) {
            auto *data = reinterpret_cast<const Byte *>(chunk);
            Dword tag =
                (Dword(data[0]) << 24) | (Dword(data[1]) << 16) | (Dword(data[2]) << 8) | data[3];
            if (is_mad_chunk(tag)) {
                auto *storage = static_cast<Chunk *>(memory.allocate("", 12, 0, 0, memory.pool));
                if (!storage) {
                    files::streaming::release_chunk(stream, chunk);
                    throw std::bad_alloc();
                }
                auto *record = construct_chunk(storage);
                *output = record;
                record->token = chunk;
                Dword bytes = chunk->size;
                record->data = chunk;
                record->bytes = bytes;
                if (!delegate->decoder) {
                    void *decoder = pending::create_mad_decoder();
                    if (!decoder) {
                        release_chunk(player, record);
                        *output = nullptr;
                        throw std::bad_alloc();
                    }
                    int status = bind_decoder(delegate, decoder, record);
                    if (status != 2) {
                        release_decoder(delegate);
                        retire_chunk(delegate, record);
                        delegate->chunk = nullptr;
                        *output = nullptr;
                        throw std::runtime_error("MAD decoder rejected the resource stream");
                    }
                }
                return;
            }
            files::streaming::release_chunk(stream, chunk);
        }
        if (files::streaming::empty(stream))
            return;
        // Authorized movie compatibility: the original queue pump has returned
        // and holds no stream/audio lock. Yield only while no chunk is ready.
        if (!chunk)
            Sleep(1);
    }
}

void __cdecl acquire_callback(Delegate *delegate, Player **player, Chunk **output) {
    acquire_chunk(*player, delegate, output);
}

void release_chunk(Player *player, Chunk *chunk) {
    files::streaming::release_chunk(player->video,
                                    static_cast<files::streaming::Chunk *>(chunk->token));
    memory.release(chunk);
}

void __cdecl release_callback(Delegate *, Player **player, Chunk *chunk) {
    release_chunk(*player, chunk);
}
} // namespace nfs::movies
