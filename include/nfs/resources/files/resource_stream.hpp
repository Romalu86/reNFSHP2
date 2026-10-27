#pragma once
#include "nfs/resources/files/filesystem.hpp"
namespace nfs::files::streaming {
using Byte = std::uint8_t;
struct Pool;
struct Channel {
    Pool *pool;
    int number;
    int buffered_bytes;
    Byte *next_chunk;
};
struct Rule {
    Dword mask, value;
    int channel;
};
struct Request {
    Dword token;
    int state;
    Request *previous, *next;
    int memory_source;
    char path[260];
    const Byte *memory;
    int length;
    Dword end_chunk;
    Byte *begin;
};
struct Pool {
    Dword magic;
    kernel::Lock lock;
    Request *requests;
    int request_count;
    Rule *rules;
    int rule_count;
    Channel *channels;
    int channel_count;
    Byte *buffer_begin, *wrap_begin, *buffer_end;
    volatile int state;
    int normal_priority, urgent_priority, watermark, urgent, buffered_bytes;
    Byte *release_cursor, *chunk_cursor, *write_cursor;
    Request *first, *current, *last, *free_requests;
    char path[260];
    Dword file;
    int file_offset;
    Dword operation;
    int read_bytes, read_quantum;
};
struct Chunk {
    Dword kind, size;
};
static_assert(sizeof(Channel) == 16 && sizeof(Rule) == 12 && sizeof(Request) == 296 &&
              sizeof(Pool) == 400);
static_assert(offsetof(Pool, requests) == 32 && offsetof(Pool, buffer_begin) == 56 &&
              offsetof(Pool, state) == 68 && offsetof(Pool, path) == 120 &&
              offsetof(Pool, file) == 380);
static_assert(offsetof(Request, memory) == 280 && offsetof(Request, end_chunk) == 288);
extern Dword request_sequence;
inline Dword address(const void *pointer) {
    return reinterpret_cast<Dword>(pointer);
}
inline Byte *pointer(Dword value) {
    return reinterpret_cast<Byte *>(value);
}
int storage_bytes(int, int, int);
Channel *initialize(int, int, int, void *, int);
int resolve(Channel *, Pool **, Channel **);
void destroy(Channel *);
void set_rule(Channel *, int, Dword, Dword, int);
void set_priorities(Channel *, int, int);
void set_watermark(Channel *, int);
void set_urgent(Channel *, int);
Channel *channel(Channel *, int);
Request *reserve_request(Pool *);
void append_request(Pool *, Request *);
void recycle_request(Pool *, Request *);
Request *find_request(Pool *, Dword);
Dword enqueue_file(Channel *, const char *, int, Dword);
Dword enqueue_memory(Channel *, const void *, int, Dword);
void cancel_request(Channel *, Dword);
void cancel_all(Channel *);
int in_ring_range(Dword, Dword, Dword);
void start_next(Pool *, int);
void file_opened(Dword, int, Dword);
void file_closed(Dword, int, Dword);
void fill_ring(Pool *, int);
void read_completed(Dword, int, Dword);
int publish_chunks(Pool *);
int route_chunk(Pool *, const Chunk *);
void account_release(Pool *, int);
Chunk *take_chunk(Channel *);
void release_chunk(Channel *, Chunk *);
Dword queued_bytes(Channel *);
int state(Channel *);
bool empty(Channel *);
int buffer_bytes(Channel *);
} // namespace nfs::files::streaming
