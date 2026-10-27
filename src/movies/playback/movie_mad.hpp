#pragma once
#include "movie_runtime.hpp"
namespace nfs::movies::mad {
struct Link {
    Link *next, *previous;
};
struct Buffer {
    Dword kind;
    void *shape;
    Link link;
    int references;
};
struct Decoder {
    const Dword *methods;
    float rate;
    Dword duration;
    int first;
    Dword width, height;
    Buffer *reference;
    Dword frame;
    Link busy, available;
    Delegate *source;
};
static_assert(sizeof(Buffer) == 20 && offsetof(Buffer, link) == 8 && sizeof(Decoder) == 52);
extern const Dword methods[6], base_methods[6];
Buffer *construct_buffer(Buffer *, Dword, Dword);
void destroy_buffer(Buffer *);
Decoder *construct(Decoder *);
void destroy(Decoder *);
void *__fastcall delete_decoder(Decoder *, void *, unsigned);
void *__fastcall delete_base(Decoder *, void *, unsigned);
int __fastcall initialize(Decoder *, void *, Delegate *, Chunk *);
void __fastcall retain(Decoder *, void *, Buffer *);
Dword __fastcall frame(Decoder *, void *);
float __fastcall frame_rate(Decoder *, void *);
void read_chunk(Decoder *, Chunk **);
void release_chunk(Decoder *, Chunk *);
Dword chunk_tag(Chunk *);
Buffer *__fastcall seek_frame(Decoder *, void *, int);
Buffer *reserve_reference(Decoder *);
Buffer *reserve_frame(Decoder *);
void __fastcall release_frame(Decoder *, void *, Buffer *);
namespace pending {
Buffer *decode_chunk(Decoder *, Chunk *);
}
inline Buffer *buffer(Link *link) {
    return reinterpret_cast<Buffer *>(reinterpret_cast<Byte *>(link) - 8);
}
inline void detach(Link *link) {
    link->previous->next = link->next;
    link->next->previous = link->previous;
}
inline void append(Link *head, Link *link) {
    Link *last = head->previous;
    head->previous = link;
    last->next = link;
    link->next = head;
    link->previous = last;
}
} // namespace nfs::movies::mad
