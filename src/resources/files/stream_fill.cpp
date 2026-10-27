#include "nfs/resources/files/resource_stream.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/movies/streams/movie_stream_events.hpp"
#include <cstring>
namespace nfs::files::streaming {

void start_next(Pool *pool, int priority) {
    kernel::lock_enter(&pool->lock);
    Request *request = pool->current;
    bool idle = true;
    if (!request)
        pool->state = 0;
    else if (request->state != 1 && !request->next)
        pool->state = 0;
    else {
        if (request->state != 1)
            pool->current = request->next;
        request = pool->current;
        request->begin = pool->chunk_cursor;
        request->state = 2;
        idle = false;
    }
    kernel::lock_leave(&pool->lock);
    if (idle)
        return;
    pool->write_cursor = pool->chunk_cursor;
    if (request->memory_source == 1) {
        pool->file_offset = 0;
        fill_ring(pool, priority);
        return;
    }
    pool->file_offset = request->length;
    if (!std::strcmp(request->path, pool->path)) {
        fill_ring(pool, priority);
        return;
    }
    std::strcpy(pool->path, request->path);
    if (!pool->file) {
        pool->operation = open(pool->path, 1, priority, address(pool));
        if (pool->operation)
            set_callback(pool->operation, file_opened);
    } else {
        pool->operation = close(pool->file, priority, address(pool));
        if (pool->operation)
            set_callback(pool->operation, file_closed);
    }
}

void file_opened(Dword operation, int status, Dword context) {
    auto *pool = reinterpret_cast<Pool *>(context);
    pool->file = complete(pool->operation);
    if (pool->file)
        fill_ring(pool, pool->urgent_priority);
    else if (movies::notify_stream_open_failed(pool, operation, status)) {
        // Explicit movie compatibility: complete() has consumed the failed
        // open. No producer remains to leave state1; do not fabricate an EOF
        // chunk or change the error behavior of unrelated game streams.
        pool->operation = 0;
        pool->state = 0;
    }
}

void file_closed(Dword, int, Dword context) {
    auto *pool = reinterpret_cast<Pool *>(context);
    complete(pool->operation);
    pool->operation = open(pool->path, 1, pool->urgent_priority, address(pool));
    if (pool->operation)
        set_callback(pool->operation, file_opened);
}

void fill_ring(Pool *pool, int priority) {
    const Byte *published_end = pool->chunk_cursor;
    while (pool->release_cursor != published_end) {
        auto *chunk = reinterpret_cast<Chunk *>(pool->release_cursor);
        if (chunk->kind == Dword(-1))
            pool->release_cursor = pool->wrap_begin;
        else if (chunk->kind == Dword(-2))
            pool->release_cursor = pointer(address(chunk) + chunk->size);
        else
            break;
    }
    kernel::lock_enter(&pool->lock);
    Request *request = pool->first;
    while (request->next && request->next->state != 1 &&
           !in_ring_range(address(pool->release_cursor), address(pool->write_cursor),
                          address(request->next->begin) - 1)) {
        recycle_request(pool, request);
        request = pool->first;
    }
    kernel::lock_leave(&pool->lock);
    Dword released = address(pool->release_cursor), write = address(pool->write_cursor);
    int available;
    if (write < released)
        available = int(released - write - 65);
    else {
        available = int(address(pool->buffer_end) - write - 64);
        if (available < pool->read_quantum) {
            Dword pending = write - address(pool->chunk_cursor);
            bool memory = pool->current->memory_source == 1;
            int head = int(released - address(pool->wrap_begin) - (memory ? 0 : 64));
            if (head < int(pending + 1)) {
                pool->state = 2;
                return;
            }
            int remainder = int(pending) % 64;
            pool->wrap_begin = (!remainder || memory)
                                   ? pool->buffer_begin
                                   : pointer(address(pool->buffer_begin) - Dword(remainder) + 64);
            kernel::memory_copy(pool->wrap_begin, pool->chunk_cursor, pending);
            auto *jump = reinterpret_cast<Chunk *>(pool->chunk_cursor);
            jump->kind = Dword(-1);
            jump->size = 8;
            pool->chunk_cursor = pool->wrap_begin;
            pool->write_cursor = pointer(address(pool->wrap_begin) + pending);
            if (reinterpret_cast<Chunk *>(pool->release_cursor)->kind == Dword(-1)) {
                pool->release_cursor = pool->wrap_begin;
                available = int(address(pool->buffer_end) - address(pool->write_cursor) - 64);
            } else
                available = int(address(pool->release_cursor) - address(pool->write_cursor) - 1);
        }
    }
    if (available < pool->read_quantum) {
        pool->state = 2;
        return;
    }
    request = pool->current;
    if (request->memory_source == 1) {
        pool->read_bytes = request->length < int(Dword(pool->file_offset) + Dword(available))
                               ? int(Dword(request->length) - Dword(pool->file_offset))
                               : available;
        kernel::memory_copy(pool->write_cursor, request->memory, Dword(pool->read_bytes));
        request->memory = pointer(address(request->memory) + Dword(pool->read_bytes));
        read_completed(0, 0, address(pool));
        return;
    }
    pool->read_bytes = pool->read_quantum;
    pool->operation = read(pool->file, pool->file_offset, pool->write_cursor, pool->read_quantum,
                           priority, address(pool));
    if (pool->operation)
        set_callback(pool->operation, read_completed);
}

void read_completed(Dword, int, Dword context) {
    auto *pool = reinterpret_cast<Pool *>(context);
    Request *request = pool->current;
    int bytes;
    bool end;
    if (request->memory_source == 1) {
        bytes = pool->read_bytes;
        end = request->length <= int(Dword(pool->file_offset) + Dword(bytes));
    } else {
        bytes = int(complete(pool->operation));
        end = bytes < pool->read_bytes;
    }
    pool->file_offset = int(Dword(pool->file_offset) + Dword(bytes));
    pool->write_cursor = pointer(address(pool->write_cursor) + Dword(bytes));
    int terminator = publish_chunks(pool);
    if (request->state == 4) {
        start_next(pool, pool->urgent_priority);
        return;
    }
    if (!end && !terminator) {
        fill_ring(pool, pool->urgent_priority);
        return;
    }
    kernel::lock_enter(&pool->lock);
    if (request->state != 4)
        request->state = 3;
    kernel::lock_leave(&pool->lock);
    start_next(pool, pool->urgent_priority);
}

int route_chunk(Pool *pool, const Chunk *chunk) {
    for (int i = 0; i < pool->rule_count; ++i)
        if ((chunk->kind & pool->rules[i].mask) == pool->rules[i].value)
            return pool->rules[i].channel;
    return -2;
}

int publish_chunks(Pool *pool) {
    auto *chunk = reinterpret_cast<Chunk *>(pool->chunk_cursor);
    Request *request = pool->current;
    if (int(address(pool->write_cursor) - address(chunk)) < 8)
        return 0;
    for (;;) {
        Dword bytes = chunk->size;
        if (bytes & 4278190080u) {
            bytes = 8;
            chunk->kind = request->end_chunk;
            chunk->size = 8;
        }
        if (address(pool->write_cursor) < address(pool->chunk_cursor) + bytes)
            return 0;
        if (chunk->kind == request->end_chunk) {
            Dword alignment = address(pool->chunk_cursor) & 63;
            bytes = ((alignment + 63 + bytes) & ~Dword(63)) - alignment;
        }
        int routed = route_chunk(pool, chunk);
        bool cancelled;
        if (routed < 0) {
            kernel::lock_enter(&pool->lock);
            cancelled = request->state == 4;
            if (!cancelled) {
                chunk->kind = Dword(-2);
                pool->chunk_cursor = pointer(address(pool->chunk_cursor) + bytes);
            }
        } else {
            chunk->size = (Dword(routed) << 24) | bytes;
            kernel::lock_enter(&pool->lock);
            cancelled = request->state == 4;
            if (!cancelled) {
                auto *channel = pool->channels + routed - 1;
                Dword queued = Dword(channel->buffered_bytes) + bytes;
                channel->buffered_bytes = int(queued);
                if (queued == bytes)
                    channel->next_chunk = reinterpret_cast<Byte *>(chunk);
                const int watermark = static_cast<volatile Pool *>(pool)->watermark;
                pool->chunk_cursor = pointer(address(pool->chunk_cursor) + bytes);
                int previous = pool->buffered_bytes;
                int current = int(Dword(previous) + bytes);
                pool->buffered_bytes = current;
                if (previous < watermark && watermark <= current)
                    pool->urgent = 0;
            }
        }
        kernel::lock_leave(&pool->lock);
        if (cancelled) {
            if (chunk->kind != request->end_chunk) {
                Dword alignment = address(pool->chunk_cursor) & 63;
                chunk->size =
                    (((alignment + 63 + bytes) & ~Dword(63)) - alignment) | (Dword(routed) << 24);
            }
            return 0;
        }
        if (chunk->kind == request->end_chunk)
            return 1;
        chunk = reinterpret_cast<Chunk *>(pool->chunk_cursor);
        if (int(address(pool->write_cursor) - address(chunk)) < 8)
            return 0;
    }
}
} // namespace nfs::files::streaming
