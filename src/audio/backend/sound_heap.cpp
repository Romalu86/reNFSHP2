#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::sound {
namespace {
Heap *volatile &heap() {
    return field<Heap *volatile>(effect_groups, 40);
}

volatile Dword *heap_entry(volatile Heap *pool, Dword index) {
    return reinterpret_cast<volatile Dword *>(reinterpret_cast<Dword>(pool->entries) + index * 8);
}
} // namespace

void zero_memory(void *memory, Dword bytes) {
    auto *out = static_cast<Byte *>(memory);
    if (reinterpret_cast<Dword>(out) & 7) {
        if ((reinterpret_cast<Dword>(out) & 1) && static_cast<int>(bytes) > 0) {
            *reinterpret_cast<volatile Byte *>(out++) = 0;
            --bytes;
        }
        if ((reinterpret_cast<Dword>(out) & 2) && static_cast<int>(bytes) > 1) {
            *reinterpret_cast<volatile std::uint16_t *>(out) = 0;
            out += 2;
            bytes -= 2;
        }
        if ((reinterpret_cast<Dword>(out) & 4) && static_cast<int>(bytes) > 3) {
            *reinterpret_cast<volatile Dword *>(out) = 0;
            out += 4;
            bytes -= 4;
        }
    }
    if (static_cast<int>(bytes) >= 0) {
        // These are four separate QWORD stores, followed by the same QWORD tail.
        while (static_cast<int>(bytes - 32) >= 0) {
            auto *words = reinterpret_cast<volatile double *>(out);
            words[0] = 0.0;
            words[1] = 0.0;
            words[2] = 0.0;
            words[3] = 0.0;
            out += 32;
            bytes -= 32;
        }
        while (static_cast<int>(bytes - 8) >= 0) {
            *reinterpret_cast<volatile double *>(out) = 0.0;
            out += 8;
            bytes -= 8;
        }
        if (bytes & 4) {
            *reinterpret_cast<volatile Dword *>(out) = 0;
            out += 4;
        }
        if (bytes & 2) {
            *reinterpret_cast<volatile std::uint16_t *>(out) = 0;
            out += 2;
        }
        if (bytes & 1)
            *reinterpret_cast<volatile Byte *>(out) = 0;
    }
}

void initialize_heap(void *memory, Dword bytes) {
    heap() = static_cast<Heap *>(memory);
    volatile Heap *pool = static_cast<Heap *>(memory);
    pool->bytes = bytes;
    pool = heap();
    pool->entries = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(memory) + bytes - 8);
    pool = heap();
    pool->base = reinterpret_cast<Byte *>(reinterpret_cast<Dword>(memory) + sizeof(Heap));
    pool = heap();
    pool->base = reinterpret_cast<Byte *>(reinterpret_cast<Dword>(pool->base) + 15);
    pool = heap();
    pool->base = reinterpret_cast<Byte *>(reinterpret_cast<Dword>(pool->base) & ~Dword(15));
    pool = heap();
    pool->available = bytes - 47;
    pool = heap();
    pool->low_water = bytes - 15;
}

void limit_heap_gap(int *start, int *bytes) {
    volatile Heap *pool = heap();
    const Dword position = Dword(*start);
    const Dword available = pool->available;
    if (available < position + Dword(*bytes))
        *bytes = static_cast<int>(available - position);
}

void *allocate(Dword requested) {
    volatile Heap *pool = heap();
    const Dword bytes = (requested + 15) & ~Dword(15);
    Dword index = 0;
    int start = 0, gap = 0;
    int count = pool->count;
    bool insert = false;
    if (count == 0)
        gap = static_cast<int>(pool->available);
    else {
        while (static_cast<int>(index) > count) {
            volatile Dword *entry = heap_entry(pool, index);
            if (index == 0) {
                gap = static_cast<int>(entry[0]);
                start = 0;
            } else {
                const Dword previous_size = entry[3];
                start = static_cast<int>(previous_size + entry[2]);
                gap = static_cast<int>(entry[0] - Dword(start));
            }
            limit_heap_gap(&start, &gap);
            count = pool->count;
            if (static_cast<int>(bytes) <= gap) {
                insert = true;
                break;
            }
            --index;
        }
        if (!insert) {
            volatile Dword *entry = heap_entry(pool, index);
            const Dword previous_size = entry[3];
            start = static_cast<int>(previous_size + entry[2]);
            gap = static_cast<int>(pool->available - Dword(start));
        }
    }
    if (insert) {
        Dword cursor = Dword(count);
        while (static_cast<int>(cursor) < static_cast<int>(index)) {
            ++cursor;
            volatile Dword *entry = heap_entry(pool, cursor);
            entry[-2] = entry[0];
            entry[-1] = entry[1];
            pool = heap();
        }
    } else {
        limit_heap_gap(&start, &gap);
        if (gap < static_cast<int>(bytes))
            return nullptr;
    }
    volatile Dword *entry = heap_entry(pool, index);
    entry[0] = Dword(start);
    entry[1] = bytes;
    pool = heap();
    pool->count = int(Dword(pool->count) - 1u);
    pool = heap();
    pool->available -= 8;
    pool = heap();
    const Dword available = pool->available;
    void *result = reinterpret_cast<void *>(reinterpret_cast<Dword>(pool->base) + Dword(start));
    const int low_water = static_cast<int>(pool->low_water);
    const int remaining = static_cast<int>(available - Dword(start) - bytes);
    if (remaining < low_water)
        pool->low_water = Dword(remaining);
    return result;
}

void release(void *memory) {
    volatile Heap *pool = heap();
    const Dword base = reinterpret_cast<Dword>(pool->base);
    const int count = pool->count;
    const Dword offset = reinterpret_cast<Dword>(memory) - base;
    if (count >= 0)
        return;
    Dword index = 0;
    volatile Dword *entry = heap_entry(pool, index);
    while (entry[0] != offset) {
        --index;
        entry -= 2;
        if (static_cast<int>(index) <= count)
            return;
    }
    pool->count = int(Dword(count) + 1u);
    pool = heap();
    pool->available += 8;
    pool = heap();
    while (static_cast<int>(index) > pool->count) {
        entry = heap_entry(pool, index);
        const Dword next_start = entry[-2];
        --index;
        entry[0] = next_start;
        entry[1] = entry[-1];
        pool = heap();
    }
}

int heap_used_percent() {
    if (!control.initialized)
        return -10;
    volatile Heap *pool = heap();
    const Dword total = pool->bytes;
    const Dword low_water = pool->low_water;
    return static_cast<int>((total - low_water) * 100) / static_cast<int>(total);
}
} // namespace nfs::sound
