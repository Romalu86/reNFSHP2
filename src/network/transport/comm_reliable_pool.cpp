#include "comm_filter_runtime.hpp"
namespace nfs::network::session::pending {
namespace {
extern const Dword reliable_pool_methods[];
}
void clear_reliable_entry(ReliableEntry *entry) {
    comm::reset_cursor(&entry->packet);
    // The native entry reset retains the packet payload and clears these fields forward.
    at<volatile Dword>(entry, 332) = 0;
    at<volatile Dword>(entry, 336) = 0;
    at<volatile Dword>(entry, 340) = 0;
    at<volatile Dword>(entry, 344) = 0;
    at<volatile Dword>(entry, 352) = 0;
    at<volatile Byte>(entry, 356) = 1;
}

Dword reset_reliable_pool(ReliablePool *owner, Dword epoch, Dword first) {
    if (!owner->entries)
        return 0;
    enter_comm_lock(owner->lock);
    owner->epoch = epoch;
    owner->write = owner->read = first;
    for (Dword i = 0; i < owner->capacity; ++i)
        clear_reliable_entry(owner->entries + i);
    leave_comm_lock(owner->lock);
    return 1;
}

ReliablePool *construct_reliable_pool(ReliablePool *owner, Dword capacity, Dword entry_bytes) {
    comm::initialize_kind(&owner->kind, 7);
    owner->methods = reliable_pool_methods;
    const Dword stride = (entry_bytes + 3) & ~Dword(3);
    owner->capacity = capacity;
    owner->stride = stride;
    const Dword bytes_count = stride * capacity;
    void *allocation = gui::application_allocate(capacity * 360 + 4);
    ReliableEntry *entries = nullptr;
    if (allocation) {
        at<Dword>(allocation, 0) = capacity;
        entries = reinterpret_cast<ReliableEntry *>(bytes(allocation) + 4);
        for (int i = 0; i < int(capacity); ++i) {
            ReliableEntry *entry = entries + i;
            comm::initialize_kind(&entry->kind, 8);
            comm::construct_packet(&entry->packet, nullptr, 0, 0);
            entry->slot = 0;
            entry->previous = entry->next = nullptr;
            clear_reliable_entry(entry);
        }
    }
    owner->entries = entries;
    owner->data = static_cast<Byte *>(comm::allocate_named("llpacket_queue table", bytes_count, 0));
    std::memset(owner->data, 0, bytes_count);
    Byte *data = owner->data;
    ReliableEntry *previous = reinterpret_cast<ReliableEntry *>(reinterpret_cast<Dword>(entries) -
                                                                360 + capacity * 360);
    for (Dword i = 0; i < capacity; ++i) {
        ReliableEntry *entry = entries + i;
        entry->slot = i;
        entry->previous = entry->next = nullptr;
        clear_reliable_entry(entry);
        comm::bind_buffer(&entry->packet, data, 0, int(stride), nullptr);
        data += stride;
        entry->next = i >= capacity - 1 ? entries : entries + i + 1;
        entry->previous = previous;
        previous = entry;
    }
    owner->lock = allocate_comm_lock();
    reset_reliable_pool(owner, comm_milliseconds(), 0);
    return owner;
}

static void destroy_reliable_pool(ReliablePool *owner) {
    owner->methods = reliable_pool_methods;
    enter_comm_lock(owner->lock);
    if (owner->entries) {
        Dword count = at<Dword>(reinterpret_cast<Byte *>(owner->entries) - 4, 0);
        for (Dword i = count; i; --i)
            comm::destroy_packet(&owner->entries[i - 1].packet);
        gui::application_release(reinterpret_cast<Byte *>(owner->entries) - 4);
    }
    kernel::memory_free(owner->data);
    leave_comm_lock(owner->lock);
    if (owner->lock)
        owner->lock->methods[0](owner->lock, 1);
}

static void *__fastcall delete_reliable_pool(ReliablePool *owner, void *, Dword flags) {
    destroy_reliable_pool(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

ReliableEntry *enqueue_reliable_packet(ReliablePool *owner, comm::Packet *packet) {
    if (!packet)
        return nullptr;
    const Dword slot = owner->write % owner->capacity;
    ReliableEntry *entry = owner->entries + slot;
    if (slot != entry->slot)
        _assert("slotID == pEntry->m_slotID", "cmn\\llpacket_queue.cpp", 167);
    if (!entry->available)
        return nullptr;
    enter_comm_lock(owner->lock);
    entry->available = 0;
    entry->retries = 0;
    entry->identifier = owner->write;
    comm::set_endpoint(&entry->packet, packet->endpoint);
    const Dword count = Dword(comm::available_bytes(packet));
    if (owner->stride < count)
        _assert("dataSize <= m_entryDataSize", "cmn\\llpacket_queue.cpp", 187);
    comm::reset_cursor(&entry->packet);
    comm::write_packet(&entry->packet, comm::packet_data(packet), int(count));
    ++owner->write;
    leave_comm_lock(owner->lock);
    return entry;
}

ReliableEntry *store_reliable_packet(ReliablePool *owner, Dword identifier, const void *data,
                                     Dword count) {
    ReliableEntry *entry = owner->entries + identifier % owner->capacity;
    if (!entry->available)
        return nullptr;
    enter_comm_lock(owner->lock);
    clear_reliable_entry(entry);
    entry->identifier = identifier;
    comm::reset_cursor(&entry->packet);
    comm::write_packet(&entry->packet, data, int(count));
    entry->available = 0;
    leave_comm_lock(owner->lock);
    return entry;
}

Dword acknowledge_reliable_packets(ReliablePool *owner, Dword identifier) {
    ReliableEntry *entry = owner->entries + owner->read % owner->capacity;
    if (identifier < entry->identifier)
        return 0;
    const Dword slot = identifier % owner->capacity;
    if (owner->entries[slot].available)
        return 0;
    enter_comm_lock(owner->lock);
    for (;;) {
        clear_reliable_entry(entry);
        if (entry == owner->entries + slot)
            break;
        entry = entry->next;
    }
    owner->read = identifier + 1;
    leave_comm_lock(owner->lock);
    return 1;
}
// Callers consume AL only.
Byte reliable_pool_empty(ReliablePool *owner) {
    for (Dword i = 0; i < owner->capacity; ++i)
        if (!owner->entries[i].available)
            return 0;
    return 1;
}
namespace {
const Dword reliable_pool_methods[]{reinterpret_cast<Dword>(delete_reliable_pool)};
}
} // namespace nfs::network::session::pending
