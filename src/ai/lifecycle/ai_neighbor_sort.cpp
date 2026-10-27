#include "nfs/physics/solver/solver_runtime.hpp"
#include <utility>
namespace nfs::game::runtime::ai {

int pair_index(int first, int second) {
    if (first < second)
        std::swap(first, second);
    // Native LEA/IMUL/ADD wrap in32 bits; only the division is signed/truncating.
    const int product = static_cast<int>((Dword(first) + 1u) * Dword(first));
    return static_cast<int>(Dword(product / 2) + Dword(second));
}

float *pair_entry(Dword *registry, int first, int second) {
    const Dword offset = 204u + Dword(pair_index(first, second)) * 8u;
    return reinterpret_cast<float *>(reinterpret_cast<std::uintptr_t>(registry) + offset);
}
// ECX points to the captured comparison owner.
Byte closer(void **context, void *first, void *second) {
    const float a = float(pair_distance(*context, first));
    const float b = float(pair_distance(*context, second));
    return a < b;
}

void insertion_sort(void **begin, void **end, void *owner) {
    if (begin == end)
        return;
    for (void **cursor = begin + 1; cursor != end; ++cursor) {
        void *value = *cursor;
        void **destination = cursor;
        const long double distance = pair_distance(owner, value);
        if (!(distance < pair_distance(owner, *begin))) {
            while (true) {
                void *previous = destination[-1];
                if (!(pair_distance(owner, value) < pair_distance(owner, previous)))
                    break;
                *destination = previous;
                --destination;
            }
            *destination = value;
        } else {
            for (; destination != begin; --destination)
                *destination = destination[-1];
            *begin = value;
        }
    }
}

void *median_vehicle(void *a, void *b, void *c, void *owner) {
    void *candidate;
    if (!(pair_distance(owner, a) < pair_distance(owner, b))) {
        if (pair_distance(owner, a) < pair_distance(owner, c))
            return a;
        candidate = b;
    } else {
        if (pair_distance(owner, b) < pair_distance(owner, c))
            return b;
        candidate = a;
    }
    return pair_distance(owner, candidate) < pair_distance(owner, c) ? c : candidate;
}
// The EAX return is the left partition boundary.
void **partition_neighbors(void **left, void **right, void *pivot, void *owner) {
    for (;;) {
        void *value = *left;
        if (!(pair_distance(owner, value) < pair_distance(owner, pivot))) {
            do {
                --right;
            } while (pair_distance(owner, pivot) < pair_distance(owner, *right));
            if (right <= left)
                return left;
            *left = *right;
            ++left;
            *right = value;
        } else
            ++left;
    }
}
// Preserve the saved 16-element cutoff and pivot tie choices.
void quick_partition_sort(void **begin, void **end, void *owner) {
    while (end - begin > 16) {
        void *first = *begin, *middle = begin[(end - begin) / 2], *last = end[-1], *pivot;
        if (!(float(pair_distance(owner, first)) < pair_distance(owner, middle))) {
            pivot = first;
            if (!(float(pair_distance(owner, first)) < pair_distance(owner, last))) {
                pivot = middle;
                if (closer(&owner, middle, last))
                    pivot = last;
            }
        } else {
            pivot = middle;
            if (!(float(pair_distance(owner, middle)) < pair_distance(owner, last))) {
                pivot = first;
                if (float(pair_distance(owner, first)) < pair_distance(owner, last))
                    pivot = last;
            }
        }
        void **left = begin, **right = end;
        for (;;) {
            void *value;
            for (;;) {
                value = *left;
                if (!(float(pair_distance(owner, value)) < pair_distance(owner, pivot)))
                    break;
                ++left;
            }
            do {
                --right;
            } while (float(pair_distance(owner, pivot)) < pair_distance(owner, *right));
            if (right <= left)
                break;
            *left = *right;
            *right = value;
            ++left;
        }
        if (left - begin < end - left) {
            quick_partition_sort(begin, left, owner);
            begin = left;
        } else {
            quick_partition_sort(left, end, owner);
            end = left;
        }
    }
}
} // namespace nfs::game::runtime::ai
