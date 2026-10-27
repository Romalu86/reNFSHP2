#include "nfs/audio/streams/audio_streams.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::sound::streaming {

void start_decoder(Stream *stream) {
    stream->voice = start_stream_voice(stream->decoder, &stream->active_format, &stream->active,
                                       &stream->voice_parameters);
    if (stream->processor[4])
        set_voice_processor(Dword(stream->voice), stream->processor);
    if (stream->channel_levels_enabled)
        set_voice_channel_levels(Dword(stream->voice), stream->channel_levels);
    stream->state = 1;
}

int parse_header(int slot, const files::streaming::Chunk *chunk) {
    auto *stream =
        static_cast<Stream *>(game::runtime::race_audio::pending::stream_slots[slot]);
    stream->current = stream->current ? stream->current->next : stream->playing.first;
    Block *block = stream->current;
    Metadata metadata;
    decode_header(0, reinterpret_cast<const Byte *>(chunk) + 8, &stream->pending_format,
                  &stream->pending, &metadata);
    block->total = metadata.samples;
    block->started = 0;
    for (unsigned index = 0; stream->pending.envelopes[index]; ++index) {
        Dword event[6];
        event[0] = 3;
        event[1] = reinterpret_cast<Dword>(stream->pending.envelopes[index]);
        event[2] = stream->pending.envelope_bytes[index];
        event[4] = block->handle;
        stream->pending.envelopes[index] = nullptr;
        stream->pending.envelope_bytes[index] = 0;
        for (int filter = 0; filter < control.filter_count; ++filter)
            reinterpret_cast<void(__cdecl *)(Dword *)>(control.filters[filter])(event);
    }
    files::streaming::release_chunk(stream->source, const_cast<files::streaming::Chunk *>(chunk));
    block->byte_rate = Dword(byte_rate(&stream->pending_format));
    if (std::memcmp(&stream->active_format, &stream->pending_format, 4) ||
        std::memcmp(&stream->active, &stream->pending, 84) || stream->pending.codec_data[0]) {
        if (stream->active_format.sample_rate) {
            stream->state = 2;
            return 0;
        }
        kernel::memory_copy(&stream->active, &stream->pending, 84);
        stream->active_format = stream->pending_format;
        stream->pending.codec_data[0] = nullptr;
    }
    if (stream->state != 1)
        start_decoder(stream);
    return 0;
}

void submit_data(Stream *stream, files::streaming::Chunk *chunk) {
    Dword packet[7];
    Dword samples = field<Dword>(chunk, 8) & 2147483647;
    unsigned channels = stream->active_format.channels;
    for (unsigned channel = 0; channel < channels; ++channel)
        packet[3 + channel] = reinterpret_cast<Dword>(chunk) + field<Dword>(chunk, 12 + channel * 4) +
                              channels * 4 + 12;
    if (samples) {
        *reinterpret_cast<files::streaming::Chunk **>(packet[3] - 4) = chunk;
        Block *block = stream->current;
        chunk->kind = block->handle;
        if (!block->started && stream->active_format.codec == 15 && samples % 1152 == 0) {
            samples -= 481;
            block->total -= 481;
        }
        block->pending += samples;
        packet[1] = samples | (Dword(block->started) << 31);
        backend::queue_stream_block(stream->decoder, packet);
        block->started = 1;
    } else
        files::streaming::release_chunk(stream->source, chunk);
}

int dispatch_chunk(int slot, files::streaming::Chunk *chunk) {
    auto *stream =
        static_cast<Stream *>(game::runtime::race_audio::pending::stream_slots[slot]);
    if (chunk->kind == 1816413011) {
        submit_data(stream, chunk);
        return 1;
    }
    if (chunk->kind == 1816675155) {
        parse_header(slot, chunk);
        return 0;
    }
    files::streaming::release_chunk(stream->source, chunk);
    return 1;
}

int should_wait(Stream *stream) {
    Block *block = stream->current;
    if (block && block->byte_rate) {
        if (block->delay < 0)
            return 1;
        if (!block->delay)
            return 0;
        Dword bytes = files::streaming::queued_bytes(stream->source);
        if (bytes > 4000000)
            bytes = 4000000;
        if ((bytes * 1000) / block->byte_rate < Dword(block->delay) &&
            files::streaming::state(stream->source) != 2) {
            if (stream->available.count > 0)
                return 1;
            if (files::streaming::state(stream->source) != 0)
                return 1;
        }
        block->delay = 0;
    }
    return 0;
}

void pump_streams() {
    lock();
    for (unsigned slot = 0; slot < backend::pending::backend_stream_slot_count; ++slot) {
        auto *stream =
            static_cast<Stream *>(game::runtime::race_audio::pending::stream_slots[slot]);
        if (!stream || !stream->playing.count)
            continue;
        if (stream->state == 2) {
            if (backend::pending::stream_samples(stream->decoder) > 0)
                continue;
            stream->active_format = stream->pending_format;
            kernel::memory_copy(&stream->active, &stream->pending, 84);
            stream->pending.codec_data[0] = nullptr;
            backend::stop_stream_queue(stream->decoder);
            start_decoder(stream);
        }
        if (should_wait(stream))
            continue;
        int remaining = 10;
        if (stream->state == 1) {
            remaining = backend::stream_free_blocks(stream->decoder);
            if (!remaining)
                continue;
        }
        int proceed = 0;
        do {
            --remaining;
            auto *chunk = files::streaming::take_chunk(stream->source);
            if (chunk)
                proceed = dispatch_chunk(int(slot), chunk);
        } while (proceed && remaining > 0);
    }
    unlock();
}
} // namespace nfs::sound::streaming
