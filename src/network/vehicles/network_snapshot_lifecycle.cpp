#include "remote_car_runtime.hpp"
namespace nfs::network::snapshots {

void reset_queue(void *owner) {
    auto *queue = static_cast<SnapshotQueue *>(owner);
    EnterCriticalSection(&queue->critical);
    for (auto &node : queue->nodes) {
        auto *words = reinterpret_cast<Dword *>(&node.state);
        for (unsigned word = 0; word < 20; ++word)
            words[word] = 0;
        node.active = 0;
        node.position_only = 0;
        node.state.tick = 4294967295u;
        node.next = nullptr;
    }
    auto *words = reinterpret_cast<Dword *>(&queue->current);
    for (unsigned word = 0; word < 20; ++word)
        words[word] = 0;
    queue->position_only = 0;
    queue->current.tick = 4294967295u;
    // These writes continue beyond the queue into the car's network motion fields.
    auto *storage = static_cast<std::uint8_t *>(owner);
    packets::at<Dword>(storage, 4568) = 0;
    queue->latest_tick = 0;
    packets::at<Dword>(storage, 4572) = 0;
    queue->tail = nullptr;
    queue->head = nullptr;
    packets::at<std::uint16_t>(storage, 4564) = 0;
    LeaveCriticalSection(&queue->critical);
}

void destroy_queue(void *owner) {
    DeleteCriticalSection(&static_cast<SnapshotQueue *>(owner)->critical);
}

void peek(void *owner, void *output) {
    if (const auto *head = static_cast<SnapshotQueue *>(owner)->head)
        copy_state(static_cast<Snapshot *>(output), &head->state);
}

Dword newest_tick(void *car) {
    return car_queue(car)->latest_tick;
}
} // namespace nfs::network::snapshots
