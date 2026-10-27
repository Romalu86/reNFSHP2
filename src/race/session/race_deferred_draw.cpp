#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "race_deferred_draw.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdlib>
namespace nfs::graphics::deferred {
using game::runtime::invoke;
using kernel::numeric::Extended;
Queue queue;

static void insert(Bucket *bucket, Drawable *object, float depth) {
    const int count = bucket->count;
    int index = 0;
    while (index < count && !(bucket->entries[index].depth < depth))
        ++index;
    for (int moved = count; moved > index; --moved)
        bucket->entries[moved] = bucket->entries[moved - 1];
    bucket->entries[index] = Entry{object, depth};
    ++bucket->count;
}

static void draw_bucket(Bucket *bucket) {
    const int count = bucket->count;
    for (int index = 0; index < count; ++index)
        invoke<void>(bucket->entries[index].object, 1);
}

static int depth_bucket(float depth) {
    const auto format = kernel::numeric::current_format();
    const int index = int(Dword((Extended(depth, format) * 0.039999999105930328369140625f).truncate_to_integer()));
    return index < 0 || index >= 11 ? 10 : index;
}

static void enqueue_into(Queue *owner, Drawable *object) {
    // Return the stored floating-point depth.
    const float depth = invoke<float>(object, 2, owner->viewport);
    if (depth < 0 || depth >= 2000.f)
        return;
    insert(&owner->buckets[depth_bucket(depth)], object, depth);
}

void draw(Queue *owner) {
    backend::set_depth_write(context, 0);
    for (int index = 10; index >= 0; --index) {
        draw_bucket(&owner->buckets[index]);
        owner->buckets[index].count = 0;
    }
    backend::set_depth_write(context, 1);
}

void enqueue(Drawable *object) {
    enqueue_into(&queue, object);
}
// The entry constructor leaves both words untouched.
static Entry *construct_entry(Entry *entry) {
    return entry;
}
// Complete destructor is RET.
static void destroy_entry(Entry *) {
}

static Bucket *construct_bucket(Bucket *bucket) {
    for (auto &entry : bucket->entries)
        construct_entry(&entry);
    bucket->count = 0;
    return bucket;
}

static void destroy_bucket(Bucket *bucket) {
    for (int index = 127; index >= 0; --index)
        destroy_entry(&bucket->entries[index]);
}

static Queue *construct_queue(Queue *owner) {
    for (auto &bucket : owner->buckets)
        construct_bucket(&bucket);
    owner->viewport = nullptr;
    return owner;
}

static void destroy_queue(Queue *owner) {
    for (int index = 10; index >= 0; --index)
        destroy_bucket(&owner->buckets[index]);
}

static void shutdown_queue() {
    destroy_queue(&queue);
}

static void initialize_queue() {
    construct_queue(&queue);
    std::atexit(shutdown_queue);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_queue, ".CRT$XCU_NFS_006902D0",
                       initialize_queue);
}
} // namespace nfs::graphics::deferred
