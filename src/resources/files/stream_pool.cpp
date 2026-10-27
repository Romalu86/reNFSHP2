#include "nfs/resources/files/resource_stream.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::files::streaming {
Dword request_sequence{};

int storage_bytes(int requests, int rules, int channels) {
    return int((Dword(channels) + 29) * 16 + Dword(requests) * 296 + Dword(rules) * 12);
}

Channel *initialize(int requests, int rules, int channels, void *memory, int bytes) {
    int available = int(Dword(bytes) - Dword(storage_bytes(requests, rules, channels)));
    if (available <= 6143 || requests <= 1 || requests >= 257 || rules <= 0 || rules >= 17 ||
        channels <= 0 || channels > rules)
        return nullptr;
    auto *pool = static_cast<Pool *>(memory);
    volatile Pool *ordered = pool;
    pool->magic = 1297241171;
    kernel::lock_init(&pool->lock);
    auto *request_begin = reinterpret_cast<Request *>(pool + 1);
    auto *rule_begin = reinterpret_cast<Rule *>(request_begin + requests);
    auto *channel_begin = reinterpret_cast<Channel *>(rule_begin + rules);
    ordered->channel_count = channels;
    ordered->rules = rule_begin;
    ordered->requests = request_begin;
    ordered->channels = channel_begin;
    ordered->request_count = requests;
    ordered->rule_count = rules;
    auto aligned = pointer((address(channel_begin + channels) & ~Dword(63)) + 64);
    ordered->buffer_begin = aligned;
    ordered->wrap_begin = aligned;
    ordered->release_cursor = aligned;
    ordered->chunk_cursor = aligned;
    ordered->write_cursor = aligned;
    ordered->buffer_end = pointer(address(memory) + Dword(bytes));
    ordered->state = 0;
    ordered->normal_priority = 150;
    ordered->urgent_priority = 50;
    ordered->watermark = 0;
    ordered->urgent = 0;
    ordered->buffered_bytes = 0;
    ordered->first = nullptr;
    ordered->current = nullptr;
    ordered->last = nullptr;
    ordered->free_requests = request_begin;
    kernel::memory_zero(pool->path, 260);
    pool->file = 0;
    pool->read_quantum = available < 16384 ? 2048 : available < 32768 ? 4096 : 8192;
    for (int i = 0; i < requests; ++i) {
        volatile Request *request = ordered->requests + i;
        request->token = Dword(i);
        request->state = 0;
        request->next = ordered->requests + i + 1;
    }
    ordered->requests[requests - 1].next = nullptr;
    for (int i = 0; i < rules; ++i) {
        volatile Rule *rule = ordered->rules + i;
        rule->mask = 0;
        rule->value = 0;
        rule->channel = 1;
    }
    for (int i = 0; i < channels; ++i) {
        volatile Channel *channel = ordered->channels + i;
        channel->pool = pool;
        channel->number = i + 1;
        channel->buffered_bytes = 0;
    }
    return ordered->channels;
}

int resolve(Channel *handle, Pool **pool, Channel **output) {
    if (!handle || handle->pool->magic != 1297241171)
        return 1;
    *output = handle;
    *pool = handle->pool;
    return 0;
}

void set_rule(Channel *handle, int index, Dword mask, Dword value, int destination) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry) || index <= 0 || index > pool->rule_count)
        return;
    if (index == pool->rule_count && (mask | value))
        return;
    if ((destination <= 0 && destination != -1 && destination != -2) ||
        destination > pool->channel_count || pool->state)
        return;
    pool->rules[index - 1] = {mask, value, destination};
}

void destroy(Channel *handle) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return;
    cancel_all(handle);
    while (pool->state == 1) {
        if (kernel::is_thread(nullptr))
            kernel::cooperate(0);
        kernel::thread_yield(0);
    }
    pool->magic = 0;
    kernel::lock_destroy(&pool->lock);
    if (pool->file)
        close_sync(pool->file, 100);
}

void set_priorities(Channel *handle, int normal, int urgent) {
    Pool *pool;
    Channel *entry;
    if (!resolve(handle, &pool, &entry)) {
        pool->normal_priority = normal;
        pool->urgent_priority = urgent;
    }
}

void set_watermark(Channel *handle, int bytes) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return;
    int previous = pool->watermark;
    pool->watermark = bytes;
    if ((pool->buffered_bytes < previous) != (pool->buffered_bytes < bytes))
        set_urgent(handle, pool->buffered_bytes < bytes);
}

void set_urgent(Channel *handle, int urgent) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry))
        return;
    pool->urgent = urgent;
    if (urgent && pool->state == 1)
        set_priority(pool->operation, Byte(pool->urgent_priority));
}

Channel *channel(Channel *handle, int index) {
    Pool *pool;
    Channel *entry;
    if (resolve(handle, &pool, &entry) || index <= 0 || index > pool->channel_count)
        return nullptr;
    return pool->channels + index - 1;
}

Request *reserve_request(Pool *pool) {
    kernel::lock_enter(&pool->lock);
    Request *request = pool->free_requests;
    if (!request) {
        kernel::lock_leave(&pool->lock);
        return nullptr;
    }
    pool->free_requests = request->next;
    request_sequence += 256;
    if (!request_sequence)
        request_sequence = 256;
    request->token = (request->token & 255) | request_sequence;
    kernel::lock_leave(&pool->lock);
    return request;
}

void append_request(Pool *pool, Request *request) {
    request->state = 1;
    request->next = nullptr;
    kernel::lock_enter(&pool->lock);
    if (!pool->last) {
        request->previous = nullptr;
        volatile Pool *ordered = pool;
        ordered->first = request;
        ordered->current = request;
        ordered->last = request;
    } else {
        request->previous = pool->last;
        pool->last->next = request;
        pool->last = request;
    }
    kernel::lock_leave(&pool->lock);
}

void recycle_request(Pool *pool, Request *request) {
    if (request == pool->first)
        pool->first = request->next;
    else
        request->previous->next = request->next;
    if (request == pool->last)
        pool->last = request->previous;
    else
        request->next->previous = request->previous;
    if (request == pool->current)
        pool->current = request->next ? request->next : request->previous;
    request->state = 0;
    request->next = pool->free_requests;
    pool->free_requests = request;
}

int in_ring_range(Dword begin, Dword end, Dword value) {
    if (end < begin) {
        if (begin <= value)
            return 1;
    } else if (value < begin)
        return 0;
    return value < end ? 1 : 0;
}

Request *find_request(Pool *pool, Dword token) {
    if (pool->request_count <= int(token & 255))
        return nullptr;
    Request *request = pool->requests + (token & 255);
    if (request->token != token)
        return nullptr;
    return request->state ? request : nullptr;
}

Dword queued_bytes(Channel *handle) {
    Pool *pool;
    Channel *entry;
    return resolve(handle, &pool, &entry) ? 0 : Dword(entry->buffered_bytes);
}

int state(Channel *handle) {
    Pool *pool;
    Channel *entry;
    return resolve(handle, &pool, &entry) ? 0 : pool->state;
}

bool empty(Channel *handle) {
    Pool *pool;
    Channel *entry;
    return !resolve(handle, &pool, &entry) && pool->state == 0 && entry->buffered_bytes == 0;
}

int buffer_bytes(Channel *handle) {
    Pool *pool;
    Channel *entry;
    return resolve(handle, &pool, &entry)
               ? 0
               : int(address(pool->buffer_end) - address(pool->buffer_begin));
}
} // namespace nfs::files::streaming
