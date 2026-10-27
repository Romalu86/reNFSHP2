#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::track {

Dword compartment_lights(Dword *object) {
    return at<Dword>(object, 88);
}
// Relocate the serialized polygon array, grid and 16-bit index lists.
void relocate_collision(Dword *object, Dword *data) {
    object[5] = 0;
    object[6] = 1;
    object[3] = reinterpret_cast<Dword>(data);
    Dword *cursor = data + 2;
    if (data[0]) {
        data[1] = reinterpret_cast<Dword>(cursor);
        cursor += data[0] * 16;
    }
    object[0] = reinterpret_cast<Dword>(cursor);
    object[1] = reinterpret_cast<Dword>(cursor + 2);
    object[2] = reinterpret_cast<Dword>(cursor + 4);
    Dword *dimensions = cursor + 4;
    const int count = static_cast<int>(dimensions[0] * dimensions[1]);
    object[4] = reinterpret_cast<Dword>(cursor + 6);
    cursor += 6 + count * 2;
    Dword *cell = reinterpret_cast<Dword *>(object[4]);
    for (int i = 0; i < count; ++i, cell += 2)
        if (at<std::uint16_t>(cell, 0)) {
            cell[1] = reinterpret_cast<Dword>(cursor);
            cursor = reinterpret_cast<Dword *>(bytes(cursor) + at<std::uint16_t>(cell, 0) * 2);
        }
}
} // namespace nfs::game::runtime::track
