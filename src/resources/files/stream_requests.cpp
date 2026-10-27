#include "nfs/resources/files/resource_stream.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::files::streaming {

Dword enqueue_file(Channel *handle, const char *path, int offset, Dword end_chunk) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return 0;
    auto *request = reserve_request(pool);
    if (!request)
        return 0;
    request->memory_source = 0;
    std::strncpy(request->path, path, 259);
    request->length = offset;
    request->end_chunk = end_chunk;
    append_request(pool, request);
    kernel::lock_enter(&pool->lock);
    int previous = pool->state;
    if (!previous)
        pool->state = 1;
    kernel::lock_leave(&pool->lock);
    if (!previous)
        start_next(pool, pool->urgent ? pool->urgent_priority : pool->normal_priority);
    return request->token;
}

Dword enqueue_memory(Channel *handle, const void *memory, int bytes, Dword end_chunk) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return 0;
    auto *request = reserve_request(pool);
    if (!request)
        return 0;
    if (!bytes) {
        auto *chunk = static_cast<const Chunk *>(memory);
        while (chunk->kind != end_chunk) {
            bytes = int(Dword(bytes) + chunk->size);
            chunk = reinterpret_cast<const Chunk *>(pointer(address(chunk) + chunk->size));
        }
        bytes = int(Dword(bytes) + chunk->size);
    }
    request->memory_source = 1;
    request->memory = static_cast<const Byte *>(memory);
    request->length = bytes;
    request->end_chunk = end_chunk;
    append_request(pool, request);
    kernel::lock_enter(&pool->lock);
    int previous = pool->state;
    if (!previous)
        pool->state = 1;
    kernel::lock_leave(&pool->lock);
    if (!previous)
        start_next(pool, 0);
    return request->token;
}

void account_release(Pool *pool, int bytes) {
    kernel::lock_enter(&pool->lock);
    int previous = pool->buffered_bytes;
    int current = int(Dword(previous) - Dword(bytes));
    pool->buffered_bytes = current;
    kernel::lock_leave(&pool->lock);
    if (pool->watermark <= previous && current < pool->watermark) {
        pool->urgent = 1;
        if (pool->state == 1)
            set_priority(pool->operation, Byte(pool->urgent_priority));
    }
}

Chunk *take_chunk(Channel *handle) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry) || !entry->buffered_bytes)
        return nullptr;
    auto *chunk = reinterpret_cast<Chunk *>(entry->next_chunk);
    Dword bytes = chunk->size & 16777215;
    chunk->size = bytes;
    kernel::lock_enter(&pool->lock);
    int remaining = int(Dword(entry->buffered_bytes) - bytes);
    entry->buffered_bytes = remaining;
    kernel::lock_leave(&pool->lock);
    if (remaining > 0) {
        auto *cursor = pointer(address(chunk) + bytes);
        while ((reinterpret_cast<Chunk *>(cursor)->size & 4278190080u) !=
               (Dword(entry->number) << 24)) {
            auto *next = reinterpret_cast<Chunk *>(cursor);
            cursor = next->kind == Dword(-1) ? pool->wrap_begin
                                            : pointer(address(cursor) + (next->size & 16777215));
        }
        entry->next_chunk = cursor;
    }
    return chunk;
}

void release_chunk(Channel *handle, Chunk *chunk) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return;
    if (address(chunk) < address(pool->wrap_begin) ||
        address(chunk) > address(pool->buffer_end) - 8 || chunk->kind == Dword(-2))
        return;
    chunk->kind = Dword(-2);
    account_release(pool, int(chunk->size));
    kernel::lock_enter(&pool->lock);
    int previous = pool->state;
    if (previous == 2)
        pool->state = 1;
    kernel::lock_leave(&pool->lock);
    if (previous == 2)
        fill_ring(pool, pool->urgent ? pool->urgent_priority : pool->normal_priority);
}

void cancel_request(Channel *handle, Dword token) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return;
    Byte *released = nullptr, *begin = nullptr, *end = nullptr;
    kernel::lock_enter(&pool->lock);
    Request *request = find_request(pool, token);
    bool discard = false;
    if (request && request->state != 4) {
        if (request->state == 1)
            recycle_request(pool, request);
        else {
            request->state = 4;
            released = pool->release_cursor;
            begin = request == pool->first ? released : request->begin;
            end = !request->next || request->next->state == 1 ? pool->chunk_cursor
                                                              : request->next->begin;
            discard = true;
        }
    }
    kernel::lock_leave(&pool->lock);
    if (discard)
        for (int i = 0; i < pool->channel_count; ++i) {
            Channel *channel = pool->channels + i;
            if (channel->buffered_bytes <= 0)
                continue;
            if (!in_ring_range(address(released), address(begin), address(channel->next_chunk))) {
                while (in_ring_range(address(begin), address(end), address(channel->next_chunk))) {
                    auto *chunk = take_chunk(channel);
                    release_chunk(channel, chunk);
                    if (channel->buffered_bytes < 1)
                        break;
                }
            } else {
                const Dword channel_tag = Dword(channel->number) << 24;
                for (Byte *cursor = begin; cursor != end;) {
                    auto *chunk = reinterpret_cast<Chunk *>(cursor);
                    if (chunk->kind == Dword(-1))
                        cursor = pool->wrap_begin;
                    else {
                        Dword bytes = chunk->size & 16777215;
                        if ((chunk->size & 4278190080u) == channel_tag) {
                            kernel::lock_enter(&pool->lock);
                            channel->buffered_bytes = int(Dword(channel->buffered_bytes) - bytes);
                            kernel::lock_leave(&pool->lock);
                            account_release(pool, int(bytes));
                            chunk->kind = Dword(-2);
                            chunk->size = bytes;
                        }
                        cursor = pointer(address(cursor) + bytes);
                    }
                }
            }
        }
}

void cancel_all(Channel *handle) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry) || !pool->last)
        return;
    while (pool->last->state == 1 || pool->last->state == 2)
        cancel_request(handle, pool->last->token);
    while (pool->first != pool->current)
        recycle_request(pool, pool->first);
    pool->current->state = 4;
    for (int i = 0; i < pool->channel_count; ++i)
        pool->channels[i].buffered_bytes = 0;
    account_release(pool, pool->buffered_bytes);
    Byte *cursor = pool->release_cursor;
    Dword last_size = 0;
    while (cursor != pool->chunk_cursor) {
        auto *chunk = reinterpret_cast<Chunk *>(cursor);
        if (chunk->kind == Dword(-1))
            cursor = pool->wrap_begin;
        else {
            chunk->kind = Dword(-2);
            last_size = chunk->size & 16777215;
            chunk->size = last_size;
            cursor = pointer(address(cursor) + last_size);
        }
    }
    if (pool->state == 2) {
        if (cursor == pool->wrap_begin) {
            Byte *begin = pool->buffer_begin;
            volatile Pool *ordered = pool;
            ordered->chunk_cursor = begin;
            ordered->wrap_begin = begin;
            ordered->state = 0;
            return;
        }
        Dword padding = 64 - (address(pool->chunk_cursor) & 63);
        if (padding == 64)
            padding = 0;
        reinterpret_cast<Chunk *>(pointer(address(pool->chunk_cursor) - last_size))->size =
            padding + last_size;
        pool->chunk_cursor = pointer(address(pool->chunk_cursor) + padding);
        pool->state = 0;
    }
}
} // namespace nfs::files::streaming
