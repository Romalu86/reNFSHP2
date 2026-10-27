#include "nfs/core/types/kernel.hpp"
#include <intrin.h>

namespace nfs::kernel {

static Dword __cdecl address_key(const void* pointer, Dword) {
    return reinterpret_cast<Dword>(pointer);
}

void queue_init(Queue* queue, QueueKey key, Dword argument) {
    lock_init(&queue->lock);
    volatile Queue* fields = queue;
    fields->count = 0;
    fields->flags = 2;
    fields->first = nullptr;
    fields->last = nullptr;
    if (key) {
        fields->key = key;
        fields->argument = argument;
    } else {
        fields->argument = argument;
        fields->key = address_key;
    }
}

void queue_clone_lock(Queue* queue, QueueKey key, Dword argument, const Queue* source) {
    // The lock image is copied forward, one DWORD at a time, including overlap.
    static_assert(sizeof(Lock) == 7 * sizeof(unsigned long));
    __movsd(reinterpret_cast<unsigned long*>(&queue->lock),
            reinterpret_cast<const unsigned long*>(&source->lock), 7);
    volatile Queue* fields = queue;
    fields->count = 0;
    fields->flags = 0;
    fields->first = nullptr;
    fields->last = nullptr;
    fields->key = key ? key : address_key;
    fields->argument = argument;
}

void queue_destroy(Queue* queue) {
    if (queue->flags & 2)
        lock_destroy(&queue->lock);
}

void queue_push_front(Queue* queue, void* pointer) {
    lock_enter(&queue->lock);
    auto* node = static_cast<QueueNode*>(pointer);
    if (node) {
        volatile Queue* fields = queue;
        volatile QueueNode* link = node;
        link->next = fields->first;
        const Dword count = static_cast<Dword>(fields->count);
        fields->first = node;
        fields->count = static_cast<int>(count + 1u);
        if (!link->next)
            fields->last = node;
        fields->flags = fields->flags | 1u;
    }
    lock_leave(&queue->lock);
}

void queue_push_back(Queue* queue, void* pointer) {
    lock_enter(&queue->lock);
    auto* node = static_cast<QueueNode*>(pointer);
    if (node) {
        volatile Queue* fields = queue;
        auto* last = fields->last;
        static_cast<volatile QueueNode*>(node)->next = nullptr;
        const Dword count = static_cast<Dword>(fields->count);
        fields->last = node;
        fields->count = static_cast<int>(count + 1u);
        if (!last)
            fields->first = node;
        else
            static_cast<volatile QueueNode*>(last)->next = node;
        fields->flags = fields->flags | 1u;
    }
    lock_leave(&queue->lock);
}

void* queue_pop(Queue* queue) {
    lock_enter(&queue->lock);
    volatile Queue* fields = queue;
    auto* node = fields->first;
    if (node) {
        if (node == fields->last) {
            fields->last = nullptr;
            fields->first = nullptr;
        } else {
            fields->first = static_cast<volatile QueueNode*>(node)->next;
        }
        fields->count = static_cast<int>(static_cast<Dword>(fields->count) - 1u);
        static_cast<volatile QueueNode*>(node)->next = nullptr;
    }
    fields->flags = fields->flags | 1u;
    lock_leave(&queue->lock);
    return node;
}

void queue_insert(Queue* queue, void* pointer) {
    lock_enter(&queue->lock);
    auto* node = static_cast<QueueNode*>(pointer);
    if (node) {
        volatile Queue* fields = queue;
        const Dword argument = fields->argument;
        const QueueKey key = fields->key;
        const Dword value = key(node, argument);
        const Dword count = static_cast<Dword>(fields->count);
        auto* current = fields->first;
        // Both snapshots precede the count store, including an aliased node.
        fields->count = static_cast<int>(count + 1u);
        QueueNode* previous = nullptr;
        while (current) {
            const Dword current_argument = fields->argument;
            const QueueKey current_key = fields->key;
            if (current_key(current, current_argument) >= value)
                break;
            previous = current;
            current = static_cast<volatile QueueNode*>(current)->next;
        }
        static_cast<volatile QueueNode*>(node)->next = current;
        if (previous)
            static_cast<volatile QueueNode*>(previous)->next = node;
        else
            fields->first = node;
        if (!current)
            fields->last = node;
        fields->flags = fields->flags | 1u;
    }
    lock_leave(&queue->lock);
}

Dword queue_remove_unlocked(Queue* queue, void* pointer) {
    auto* node = static_cast<QueueNode*>(pointer);
    if (!node)
        return 0;
    volatile Queue* fields = queue;
    const Dword count = static_cast<Dword>(fields->count);
    if (!count)
        return 0;
    auto* previous = fields->first;
    if (previous == node) {
        auto* last = fields->last;
        fields->count = static_cast<int>(count - 1u);
        if (last == node) {
            fields->first = nullptr;
            fields->last = nullptr;
        } else {
            fields->first = static_cast<volatile QueueNode*>(node)->next;
        }
        static_cast<volatile QueueNode*>(node)->next = nullptr;
        fields->flags = fields->flags | 1u;
        return 1;
    }

    auto* current = static_cast<volatile QueueNode*>(previous)->next;
    while (current && current != node) {
        previous = current;
        current = static_cast<volatile QueueNode*>(current)->next;
    }
    if (!current)
        return 0;
    current = static_cast<volatile QueueNode*>(previous)->next;
    if (!current || current != node)
        return 0;
    fields->count = static_cast<int>(count - 1u);
    static_cast<volatile QueueNode*>(previous)->next =
        static_cast<volatile QueueNode*>(node)->next;
    if (fields->last == node)
        fields->last = previous;
    static_cast<volatile QueueNode*>(node)->next = nullptr;
    fields->flags = fields->flags | 1u;
    return 1;
}

Dword queue_remove(Queue* queue, void* node) {
    lock_enter(&queue->lock);
    Dword result = 0;
    if (node)
        result = queue_remove_unlocked(queue, node);
    lock_leave(&queue->lock);
    return result;
}

void* queue_find(Queue* queue, QueueMatch match, Dword argument) {
    lock_enter(&queue->lock);
    auto* node = queue->first;
    while (node && match && !match(node, argument))
        node = node->next;
    lock_leave(&queue->lock);
    return node;
}

void* queue_extract(Queue* queue, QueueMatch match, Dword argument) {
    lock_enter(&queue->lock);
    auto* node = queue->first;
    while (node && match && !match(node, argument))
        node = node->next;
    if (node && !queue_remove_unlocked(queue, node))
        node = nullptr;
    lock_leave(&queue->lock);
    return node;
}

void* queue_find_key(Queue* queue, Dword key) {
    lock_enter(&queue->lock);
    auto* node = queue->first;
    while (node && queue->key(node, queue->argument) < key)
        node = node->next;
    if (node && queue->key(node, queue->argument) != key)
        node = nullptr;
    lock_leave(&queue->lock);
    return node;
}

void* queue_extract_key(Queue* queue, Dword key) {
    lock_enter(&queue->lock);
    auto* node = queue->first;
    while (node && queue->key(node, queue->argument) < key)
        node = node->next;
    if (!node || queue->key(node, queue->argument) != key || !queue_remove_unlocked(queue, node))
        node = nullptr;
    lock_leave(&queue->lock);
    return node;
}

int queue_visit(Queue* queue, QueueMatch visit, Dword argument) {
    Dword count = 0;
    lock_enter(&queue->lock);
    volatile Queue* fields = queue;
    const Dword old_flags = fields->flags;
    fields->flags = old_flags & ~1u;
    lock_leave(&queue->lock);

    auto* node = fields->first;
    const auto* low_flags = reinterpret_cast<const volatile std::uint8_t*>(&queue->flags);
    while (node && !(*low_flags & 1)) {
        if (!visit(node, argument))
            fields->flags = fields->flags | 1u;
        node = static_cast<volatile QueueNode*>(node)->next;
        ++count;
    }
    lock_enter(&queue->lock);
    const Dword flags = fields->flags;
    const int result = (flags & 1u) ? static_cast<int>(count) : -1;
    fields->flags = flags | (old_flags & 1u);
    lock_leave(&queue->lock);
    return result;
}

int queue_visit_locked(Queue* queue, QueueMatch visit, Dword argument) {
    lock_enter(&queue->lock);
    int result = queue_visit(queue, visit, argument);
    lock_leave(&queue->lock);
    return result;
}

} // namespace nfs::kernel
