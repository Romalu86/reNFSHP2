#include "nfs/network/vehicles/network_snapshots.hpp"
namespace nfs::network::snapshots {
using namespace packets;
// Both stores are observable when the packed input overlaps the output.
// Keep the first positive F32 store before the optional retained negation.
namespace {
void store_component(volatile float *destination, Dword value, bool negative) {
    const long double coordinate = static_cast<long double>(static_cast<int>(value)) * 0.0000152587890625f;
    *destination = static_cast<float>(coordinate);
    if (negative)
        *destination = static_cast<float>(-coordinate);
}
} // namespace

void unpack_position(const void *source, float *destination) {
    auto *packed = const_cast<void *>(source);
    for (unsigned index = 0; index < 3; ++index) {
        const auto high = at<volatile std::uint16_t>(packed, index * 2);
        const Dword value = (((high & 32767) << 8) | at<Byte>(packed, 6 + index)) << 8;
        store_component(destination + index, value, (high & 32768) != 0);
    }
}

void unpack_velocity(const void *source, float *destination) {
    auto *packed = const_cast<void *>(source);
    // The first WORD supplies both signs. Y's magnitude is read after X is stored.
    const auto first_components = at<volatile std::uint16_t>(packed, 0);
    Dword value = (((first_components & 127) << 8) | at<Byte>(packed, 3)) << 8;
    store_component(destination, value, (first_components & 128) != 0);

    value = (((at<Byte>(packed, 1) & 127) << 8) | at<Byte>(packed, 4)) << 8;
    store_component(destination + 1, value, (first_components & 32768) != 0);

    const auto last_component = at<volatile std::uint16_t>(packed, 2);
    value = (((last_component & 127) << 8) | at<Byte>(packed, 5)) << 8;
    store_component(destination + 2, value, (last_component & 128) != 0);
}
} // namespace nfs::network::snapshots
