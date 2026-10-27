#include "nfs/audio/streams/audio_streams.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::sound::streaming {

void initialize_queue(BlockQueue *queue) {
    static_cast<Block *volatile &>(queue->first) = nullptr;
    static_cast<Block *volatile &>(queue->last) = nullptr;
    static_cast<volatile int &>(queue->count) = 0;
}

void push_front(BlockQueue *queue, Block *block) {
    Block *first = static_cast<Block *volatile &>(queue->first);
    static_cast<Block *volatile &>(block->previous) = nullptr;
    static_cast<Block *volatile &>(block->next) = first;
    first = static_cast<Block *volatile &>(queue->first);
    if (first)
        static_cast<Block *volatile &>(first->previous) = block;
    else
        static_cast<Block *volatile &>(queue->last) = block;
    static_cast<Block *volatile &>(queue->first) = block;
    queue->count = int(Dword(queue->count) + 1);
}

void push_back(BlockQueue *queue, Block *block) {
    static_cast<Block *volatile &>(block->next) = nullptr;
    static_cast<Block *volatile &>(block->previous) = static_cast<Block *volatile &>(queue->last);
    Block *last = static_cast<Block *volatile &>(queue->last);
    if (last)
        static_cast<Block *volatile &>(last->next) = block;
    else
        static_cast<Block *volatile &>(queue->first) = block;
    static_cast<Block *volatile &>(queue->last) = block;
    queue->count = int(Dword(queue->count) + 1);
}
// EAX retains the removed node in the body.
Block *pop_front(BlockQueue *queue) {
    Block *block = queue->first;
    if (block) {
        queue->first = block->next;
        if (queue->first)
            queue->first->previous = nullptr;
        else
            queue->last = nullptr;
        queue->count = int(Dword(queue->count) - 1);
    }
    return block;
}

void remove(BlockQueue *queue, Block *block) {
    if (block == queue->first)
        queue->first = queue->first->next;
    if (block == queue->last)
        queue->last = queue->last->previous;
    if (block->previous)
        block->previous->next = block->next;
    if (block->next)
        block->next->previous = block->previous;
    queue->count = int(Dword(queue->count) - 1);
}

Block *find_block(Dword handle) {
    if (int(handle) < 0)
        return nullptr;
    auto *stream = lookup(handle & 255);
    if (!stream)
        return nullptr;
    auto *block = stream->playing.first;
    while (block && block->handle != handle)
        block = block->next;
    return block;
}

void retire_block(Dword handle) {
    auto *stream = static_cast<Stream *>(
        game::runtime::race_audio::pending::stream_slots[handle & 255]);
    Block *block = find_block(handle);
    remove(&stream->playing, block);
    push_front(&stream->available, block);
    if (stream->current == block)
        stream->current = nullptr;
}
// The second callback argument is replaced by the owning chunk.
void block_released(Dword payload, Dword) {
    auto *chunk = *(reinterpret_cast<files::streaming::Chunk **>(payload) - 1);
    Dword slot = chunk->kind & 255;
    auto *stream =
        static_cast<Stream *>(game::runtime::race_audio::pending::stream_slots[slot]);
    files::streaming::release_chunk(stream->source, chunk);
}

void samples_consumed(Dword, Dword samples, Dword context) {
    auto *stream = reinterpret_cast<Stream *>(context);
    Block *block = stream->playing.first;
    int visits = 1;
    for (;;) {
        Dword consumed = samples, remainder = 0;
        if (block->pending < samples) {
            remainder = samples - block->pending;
            consumed = samples - remainder;
        }
        samples = remainder;
        block->consumed += consumed;
        block->pending -= consumed;
        if (block->total <= block->consumed)
            retire_block(block->handle);
        if (!samples)
            return;
        block = stream->playing.first;
        if (++visits > 200) {
            retire_block(block->handle);
            return;
        }
    }
}

int byte_rate(const Format *format) {
    Dword product = Dword(format->sample_rate) * format->channels;
    switch (format->codec) {
    case 10:
        return int(product * 136) >> 8;
    case 4:
        return int(product * 51) >> 8;
    case 9:
        return int(product * 256) >> 8;
    case 8:
        return int(product * 512) >> 8;
    case 14:
        return format->channels * 12;
    case 15:
    case 16:
        return format->channels * 8;
    default:
        return 0;
    }
}

int live_count() {
    int count = 0;
    for (unsigned i = 0; i < backend::pending::backend_stream_slot_count; ++i)
        if (game::runtime::race_audio::pending::stream_slots[i])
            ++count;
    return count;
}
} // namespace nfs::sound::streaming
