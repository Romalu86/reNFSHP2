#include "nfs/network/vehicles/network_snapshots.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include <cstring>
namespace nfs::network::packets::pending {
namespace {
unsigned active_snapshots(void *queue) {
    unsigned count = 0;
    for (unsigned index = 0; index < 48; ++index)
        if (at<Dword>(queue, 220 + index * 92))
            ++count;
    return count;
}
void receive_snapshot(void *car, void *payload, bool position_only) {
    auto *queue = bytes(car) + 3376;
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(queue));
    const Dword timestamp = at<Dword>(payload, 0);
    if (at<Dword>(queue, 24) == 4294967295u || at<Dword>(queue, 24) <= timestamp) {
        const unsigned count = active_snapshots(queue);
        if (count && count > 47) {
            if (!position_only) {
                alignas(4) Byte discarded[80];
                snapshots::pop(queue, discarded);
            } else {
                if (void *head = at<void *>(queue, 4556)) {
                    at<Dword>(head, 84) = 0;
                    at<void *>(queue, 4556) = at<void *>(head, 80);
                }
                if (!at<void *>(queue, 4556))
                    at<void *>(queue, 4560) = nullptr;
            }
        }
        void *before = nullptr;
        if (count)
            for (void *node = at<void *>(queue, 4556); node; node = at<void *>(node, 80))
                if (timestamp < at<Dword>(node, 0)) {
                    before = node;
                    break;
                }
        if (before)
            snapshots::insert(queue, payload, before, position_only);
        else
            snapshots::append(queue, payload, position_only);
        if (timestamp != 4294967295u && at<volatile Dword>(queue, 4552) < timestamp)
            at<volatile Dword>(queue, 4552) = timestamp;
    }
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(queue));
}
} // namespace

void receive_car_state(Dword *owner, void *packet) {
    const Byte slot = at<Byte>(packet, 1);
    if (at<std::int16_t>(owner, 10) <= 3 || !at<Dword>(owner, 12) || slot >= 24)
        return;
    auto *signal = reinterpret_cast<kernel::Signal *>(bytes(owner) + 84);
    if (!kernel::signal_ready(signal))
        return;
    if (void *car = vehicle(slot)) {
        alignas(4) Byte packed[36];
        std::memcpy(packed, bytes(packet) + 4, sizeof(packed));
        alignas(4) Byte payload[80];
        float quaternion[4];
        for (unsigned index = 0; index < 4; ++index)
            quaternion[index] =
                static_cast<float>(int(at<std::int16_t>(packed, 26 + index * 2)) * 4) * 0.0000152587890625f;
        game::runtime::physics::quaternion_to_matrix(quaternion,
                                                    reinterpret_cast<float *>(payload + 16));
        snapshots::unpack_position(packed + 4, reinterpret_cast<float *>(payload + 4));
        snapshots::unpack_velocity(packed + 14, reinterpret_cast<float *>(payload + 52));
        snapshots::unpack_velocity(packed + 20, reinterpret_cast<float *>(payload + 64));
        at<Dword>(payload, 0) = at<Dword>(packed, 0);
        at<std::uint16_t>(payload, 76) ^= std::uint16_t(
            (at<std::uint16_t>(payload, 76) ^ at<std::uint16_t>(packed, 34)) & 16383);
        receive_snapshot(car, payload, false);
    }
    kernel::signal_set(signal);
}

void receive_position(Dword *owner, void *packet) {
    const Byte slot = at<Byte>(packet, 1);
    if (at<std::int16_t>(owner, 10) <= 3 || !at<Dword>(owner, 12) || slot >= 24)
        return;
    auto *signal = reinterpret_cast<kernel::Signal *>(bytes(owner) + 84);
    if (!kernel::signal_ready(signal))
        return;
    if (void *car = vehicle(slot)) {
        alignas(4) Byte payload[80]{};
        std::memcpy(payload, bytes(packet) + 4, 16);
        receive_snapshot(car, payload, true);
    }
    kernel::signal_set(signal);
}
} // namespace nfs::network::packets::pending
