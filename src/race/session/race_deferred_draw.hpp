#pragma once
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::graphics::deferred {
struct Drawable {
    const Dword *methods;
    void *owner;
};
struct Entry {
    Drawable *object;
    float depth;
};
struct Bucket {
    Entry entries[128];
    int count;
};
struct Queue {
    Bucket buckets[11];
    void *viewport;
};
static_assert(sizeof(Entry) == 8 && sizeof(Bucket) == 1028 && sizeof(Queue) == 11312);
extern Queue queue;
void enqueue(Drawable *);
void draw(Queue *);
}
