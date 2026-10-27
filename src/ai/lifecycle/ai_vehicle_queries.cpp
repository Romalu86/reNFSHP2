#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/retained_ai.hpp"
#include "../../race/rules/race_rules_x87.hpp"
#include <utility>
#include <cfloat>
namespace nfs::game::runtime::ai {
int kind(void *vehicle) {
    auto *base = bytes(vehicle) + 4;
    const auto *offsets = *reinterpret_cast<const int **>(base);
    return invoke<int>(base + offsets[1], 0);
}

void *racer(void *vehicle) {
    return kind(vehicle) == 2 ? vehicle : nullptr;
}

void *police(void *vehicle) {
    return kind(vehicle) == 5 ? vehicle : nullptr;
}

float *pair_state(void *first, void *second) {
    int row = at<int>(first, 216);
    if (row < 0)
        return nullptr;
    int column = at<int>(second, 216);
    if (column < 0)
        return nullptr;
    if (row < column)
        std::swap(row, column);
    const int product = static_cast<int>((Dword(row) + 1u) * Dword(row));
    const Dword index = Dword(product / 2) + Dword(column);
    const Dword offset = 204u + index * 8u;
    return reinterpret_cast<float *>(
        reinterpret_cast<std::uintptr_t>(network::world::vehicle_registry) + offset);
}

long double pair_distance(void *first, void *second) {
    float *entry = pair_state(first, second);
    return entry ? entry[1] : FLT_MAX;
}

kernel::numeric::Extended retained_priority(void *vehicle) {
    unsigned char value[10];
    race_progress_x87::priority_native80(vehicle, value);
    return kernel::numeric::Extended::from_native80(value, kernel::numeric::current_format());
}
// Compatibility ABI for unreviewed callers; retained native consumers use the helper above.
long double priority(void *vehicle) {
    return double(retained_priority(vehicle));
}

void reset_pair_clock(Dword *registry) {
    at<Dword>(registry, 10004) = 0;
    at<Dword>(registry, 10008) = 0;
}

void reset_pursuit_clock(Dword *owner) {
    at<Dword>(owner, 40) = 0;
}
} // namespace nfs::game::runtime::ai
namespace nfs::network::world {

std::uint8_t iterator_done(Iterator *iterator) {
    return iterator->index >= iterator->array[0];
}
} // namespace nfs::network::world
