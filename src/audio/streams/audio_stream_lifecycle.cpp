#include "nfs/audio/streams/audio_streams.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::sound::streaming {

int create(const VoiceParameters *parameters, int blocks, int packets, void *memory, int bytes,
           files::streaming::Channel *source, int borrowed) {
    if (!control.initialized)
        return -10;
    for (unsigned slot = 0; slot < backend::pending::backend_stream_slot_count; ++slot) {
        if (game::runtime::race_audio::pending::stream_slots[slot])
            continue;
        auto *stream = static_cast<Stream *>(memory);
        zero_memory(stream, 288);
        auto *nodes = reinterpret_cast<Block *>(stream + 1);
        void *decoder_memory = nodes + blocks;
        const Dword decoder_bytes = Dword(backend::stream_total_bytes(packets));
        auto *file_memory = static_cast<Byte *>(decoder_memory) + decoder_bytes;
        int file_bytes = int(Dword(bytes) - 288 - Dword(blocks) * 40 - decoder_bytes);
        initialize_queue(&stream->playing);
        initialize_queue(&stream->available);
        for (int i = 0; i < blocks; ++i)
            push_front(&stream->available, nodes + i);
        stream->decoder = backend::create_stream_queue(
            reinterpret_cast<Dword>(block_released), reinterpret_cast<Dword>(samples_consumed),
            reinterpret_cast<Dword>(stream), decoder_memory, backend::stream_total_bytes(packets));
        if (stream->decoder < 0)
            return stream->decoder;
        if (!borrowed) {
            stream->source =
                files::streaming::initialize(int(Dword(blocks) + 2u), 1, 1, file_memory, file_bytes);
            if (!stream->source) {
                backend::release_stream_queue(stream->decoder);
                return -9;
            }
            stream->borrowed_source = 0;
            // The call passes the resource-channel pointer to the audio-slot API before registration.
             
            set_prebuffer(reinterpret_cast<Dword>(stream->source),
                          files::streaming::buffer_bytes(stream->source) / 3);
        } else {
            stream->borrowed_source = 1;
            stream->source = source;
        }
        stream->generation = 0;
        stream->voice = -1;
        kernel::memory_copy(&stream->voice_parameters, parameters, 24);
        stream->channel_levels_enabled = 0;
        if (!live_count()) {
            add_pump(pump_streams);
            control.shutdown[6] = reinterpret_cast<Callback>(shutdown);
        }
        game::runtime::race_audio::pending::stream_slots[slot] = stream;
        stream->pending.codec_data[0] = nullptr;
        stop_stream(slot);
        return int(slot);
    }
    return -9;
}
// EAX is the unmodified result of the create call.
int create_file_stream(const VoiceParameters *parameters, int blocks, int packets, void *memory,
                       int bytes) {
    return create(parameters, blocks, packets, memory, bytes, nullptr, 0);
}

Dword enqueue(Dword handle, int delay, const void *source, int offset, int memory_source) {
    if (!control.initialized)
        return Dword(-10);
    Stream *stream = lookup(handle);
    if (!stream)
        return Dword(-8);
    if (!stream->available.count)
        return Dword(-13);
    Dword request = Dword(offset);
    if (!memory_source)
        request = files::streaming::enqueue_file(stream->source, static_cast<const char *>(source),
                                                 offset, 1816478547);
    else if (memory_source == 1)
        request = files::streaming::enqueue_memory(stream->source, source, 0, 1816478547);
    if (!request)
        return Dword(-1);
    lock();
    Block *block = pop_front(&stream->available);
    zero_memory(block, 40);
    push_back(&stream->playing, block);
    block->request = request;
    stream->generation += 256;
    if (int(stream->generation) < 0)
        stream->generation = 0;
    block->handle = stream->generation | handle;
    block->delay = delay;
    Dword result = block->handle;
    unlock();
    return result;
}

Dword enqueue_file(Dword handle, int delay, const char *path, int offset) {
    if (!control.initialized)
        return Dword(-10);
    return enqueue(handle, delay, path, offset, 0);
}

int stop_stream(Dword handle) {
    if (!control.initialized)
        return -10;
    lock();
    Stream *stream = lookup(handle);
    if (!stream) {
        unlock();
        return -8;
    }
    if (stream->voice >= 0)
        backend::stop_stream_queue(stream->decoder);
    stream->voice = -1;
    if (!stream->borrowed_source)
        files::streaming::cancel_all(stream->source);
    if (stream->pending.codec_data[0])
        for (unsigned channel = 0; channel < stream->pending_format.channels; ++channel)
            release(stream->pending.codec_data[channel]);
    while (Block *block = pop_front(&stream->playing))
        push_front(&stream->available, block);
    stream->current = nullptr;
    stream->state = 0;
    zero_memory(&stream->active_format, 4);
    zero_memory(&stream->pending_format, 4);
    zero_memory(&stream->active, 84);
    zero_memory(&stream->pending, 84);
    unlock();
    return 0;
}

int destroy_stream(Dword handle) {
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    stop_stream(handle);
    if (live_count() == 1) {
        remove_pump(pump_streams);
        control.shutdown[6] = nullptr;
    }
    backend::release_stream_queue(stream->decoder);
    Byte borrowed = stream->borrowed_source;
    auto *source = stream->source;
    game::runtime::race_audio::pending::stream_slots[handle] = nullptr;
    if (!borrowed)
        files::streaming::destroy(source);
    return 0;
}

int shutdown() {
    for (unsigned slot = 0; slot < backend::pending::backend_stream_slot_count; ++slot)
        destroy_stream(slot);
    return 0;
}

int set_prebuffer(Dword handle, int bytes) {
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    files::streaming::set_watermark(stream->source, bytes);
    return 0;
}

int owned_bytes(int blocks, int packets) {
    return int(Dword(backend::stream_total_bytes(packets)) + 288 + Dword(blocks) * 40);
}

int file_stream_bytes(int blocks, int packets) {
    return int(Dword(owned_bytes(blocks, packets)) +
               Dword(files::streaming::storage_bytes(int(Dword(blocks) + 2u), 1, 1)));
}
} // namespace nfs::sound::streaming
