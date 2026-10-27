#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cfloat>
namespace nfs::game::runtime::ai::pending {
// Pair updates, neighbor lists/sort, then alternating callbacks.
void advance_pairs(Dword *registry) {
    for (int row = 0; row < static_cast<int>(registry[0]); ++row) {
        for (int column = 0; column <= row; ++column) {
            float *pair = pair_entry(registry, row, column);

            void *vehicle = at<void *>(registry, 4 + row * 4);
            if (!at<Byte>(vehicle, 212)) {
                pair[0] = FLT_MAX;
                pair[1] = FLT_MAX;
                continue;
            }
            void *other = at<void *>(registry, 4 + column * 4);
            if (!at<Byte>(other, 212)) {
                pair[0] = FLT_MAX;
                pair[1] = FLT_MAX;
            } else {
                const float *other_position = invoke<const float *>(other, 0);
                const float *position = invoke<const float *>(vehicle, 0);
                store_pair_distances(pair, position, other_position);
            }
        }
    }
    const int count = static_cast<int>(registry[0]);
    for (int i = 0; i < count; ++i) {
        void *vehicle = at<void *>(registry, 4 + i * 4);
        at<Dword>(vehicle, 8) = 0;
        if (!at<Byte>(vehicle, 212))
            continue;
        Dword *all = network::world::vehicle_registry;
        const Dword all_count = all[0];
        for (Dword j = 0; j < all_count; ++j) {
            void *other = reinterpret_cast<void *>(all[1 + j]);
            if (other != vehicle) {
                at<void *>(vehicle, 12 + at<Dword>(vehicle, 8) * 4) = other;
                ++at<Dword>(vehicle, 8);
            }
        }
        auto **begin = reinterpret_cast<void **>(bytes(vehicle) + 12);
        void **end = begin + at<Dword>(vehicle, 8);
        if (end - begin <= 16)
            insertion_sort(begin, end, vehicle);
        else {
            void **left = begin, **right = end;
            do {
                void *pivot = median_vehicle(*left, left[(right - left) / 2], right[-1], vehicle);
                void **partition = partition_neighbors(left, right, pivot, vehicle);
                if (partition - left < right - partition)
                    quick_partition_sort(left, partition, vehicle);
                else {
                    quick_partition_sort(partition, right, vehicle);
                    right = partition;
                    partition = left;
                }
                left = partition;
            } while (right - left > 16);
            insertion_sort(begin, begin + 16, vehicle);
            for (void **cursor = begin + 16; cursor != end; ++cursor) {
                void *value = *cursor;
                void **destination = cursor;
                const float distance = float(pair_distance(vehicle, value));
                while (pair_distance(vehicle, destination[-1]) > distance) {
                    *destination = destination[-1];
                    --destination;
                }
                *destination = value;
            }
        }
    }
    Dword parity = at<Dword>(registry, 10004);
    const int callback_count = static_cast<int>(registry[0]);
    for (int i = 0; i < callback_count; ++i)
        if ((parity & 1 & Dword(i)) == (parity & 1))
            invoke(at<void *>(registry, 4 + i * 4), 3);
    ++at<Dword>(registry, 10004);
}
} // namespace nfs::game::runtime::ai::pending
