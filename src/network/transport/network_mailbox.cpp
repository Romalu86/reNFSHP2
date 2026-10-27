#include "nfs/network/transport/network_mailbox.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network::comm {
// The vptr/next are intentionally not initialized here.
void initialize_entry(Entry *entry, Dword identifier, const void *data, Dword bytes,
                      const char *address, std::uint16_t port) {
    entry->identifier = identifier;
    entry->bytes = bytes;
    if (data && bytes)
        kernel::memory_copy(entry + 1, data, bytes);
    entry->span = entry->bytes + 28;
    if (!address)
        entry->endpoint = nullptr;
    else {
        entry->endpoint =
            reinterpret_cast<std::uint8_t *>(entry) + 28 + ((entry->bytes + 3) & ~Dword(3));
        std::strncpy(reinterpret_cast<char *>(entry->endpoint + 2), address, 256);
        *reinterpret_cast<std::uint16_t *>(entry->endpoint) = port;
        entry->span += 258;
    }
    entry->active = 1;
    entry->span = (entry->span + 3) & ~Dword(3);
}

Dword entry_bytes(Dword bytes, const char *address) {
    return bytes + (address ? 286 : 28);
}

void *entry_data(Entry *entry) {
    return entry->bytes ? entry + 1 : nullptr;
}

const char *entry_address(Entry *entry) {
    return entry->endpoint ? reinterpret_cast<char *>(entry->endpoint + 2) : nullptr;
}
// The saved getter retains the high half of endpoint address.
Dword entry_port(Entry *entry) {
    Dword pointer = reinterpret_cast<Dword>(entry->endpoint);
    return pointer ? (pointer & 4294901760u) | *reinterpret_cast<std::uint16_t *>(pointer)
                   : pointer & 4294901760u;
}

Entry *entry_next(Entry *entry) {
    return entry->next;
}

void destroy_entry(Entry *entry) {
    entry->methods = entry_methods;
    entry->active = 0;
}

void *__fastcall delete_entry(Entry *entry, void *, unsigned flags) {
    destroy_entry(entry);
    if (flags & 1)
        gui::application_release(entry);
    return entry;
}
const gui::Destroy entry_methods[]{reinterpret_cast<gui::Destroy>(delete_entry)};

Mailbox *construct_mailbox(Mailbox *mailbox, Dword bytes) {
    initialize_kind(reinterpret_cast<Dword *>(mailbox) + 1, 11);
    mailbox->methods = mailbox_methods;
    auto *lock = static_cast<LockObject *>(allocate_object(sizeof(LockObject)));
    if (lock) {
        initialize_kind(reinterpret_cast<Dword *>(lock) + 1, 5);
        lock->methods = lock_methods;
        fill(&lock->lock, 0, sizeof(kernel::Lock));
        kernel::lock_init(&lock->lock);
    }
    mailbox->lock = lock;
    mailbox->capacity = bytes < 1310 ? 1310 : bytes;
    mailbox->allocated_bytes = mailbox->capacity + 28;
    mailbox->buffer =
        static_cast<std::uint8_t *>(allocate_named("mail box buffer", mailbox->allocated_bytes, 0));
    fill(mailbox->buffer, 0, mailbox->allocated_bytes);
    mailbox->head = nullptr;
    mailbox->tail = nullptr;
    mailbox->count = 0;
    return mailbox;
}

Dword available(Mailbox *mailbox, std::uint8_t **end) {
    auto *position = mailbox->buffer;
    Dword bytes = mailbox->capacity;
    if (mailbox->tail && mailbox->head) {
        position = reinterpret_cast<std::uint8_t *>(mailbox->tail) + mailbox->tail->span;
        bytes = mailbox->capacity - reinterpret_cast<Dword>(position) +
                reinterpret_cast<Dword>(mailbox->buffer);
        if (mailbox->capacity < bytes)
            _assert("0", "cmn\\mailbox.cpp", 290);
    }
    if (end)
        *end = position;
    return bytes;
}
// The x87 comparison compares two zero-extended uint32 values.
Entry *append(Mailbox *mailbox, Dword identifier, const void *data, Dword bytes, const char *address,
              std::uint16_t port) {
    kernel::lock_enter(&mailbox->lock->lock);
    Entry *head = mailbox->head;
    auto *position = mailbox->tail
                         ? reinterpret_cast<std::uint8_t *>(mailbox->tail) + mailbox->tail->span
                         : mailbox->buffer;
    const Dword offset =
        (reinterpret_cast<Dword>(position) - reinterpret_cast<Dword>(mailbox->buffer) + 3) & ~Dword(3);
    const Dword remaining = mailbox->capacity - offset;
    Entry *result = nullptr;
    if (entry_bytes(bytes, address) <= remaining) {
        result = reinterpret_cast<Entry *>(position);
        initialize_entry(result, identifier, data, bytes, address, port);
        if (!mailbox->tail)
            mailbox->tail = result;
        else
            mailbox->tail->next = result;
        result->next = nullptr;
        if (!head)
            mailbox->head = mailbox->tail;
        mailbox->tail = result;
        ++mailbox->count;
    } else
        set_error(22);
    kernel::lock_leave(&mailbox->lock->lock);
    return result;
}
// Only the removed payload is zeroed; its header survives.
Dword pop(Mailbox *mailbox) {
    kernel::lock_enter(&mailbox->lock->lock);
    if (Entry *old = mailbox->head) {
        mailbox->head = entry_next(old);
        if (!mailbox->head)
            mailbox->tail = nullptr;
        fill(entry_data(old), 0, old->bytes);
        --mailbox->count;
    }
    kernel::lock_leave(&mailbox->lock->lock);
    return 1;
}

Dword clear(Mailbox *mailbox) {
    if (mailbox->head)
        fill(mailbox->buffer, 0, mailbox->allocated_bytes);
    mailbox->count = 0;
    mailbox->head = nullptr;
    mailbox->tail = nullptr;
    return 1;
}

void destroy_mailbox(Mailbox *mailbox) {
    mailbox->methods = mailbox_methods;
    clear(mailbox);
    if (mailbox->buffer) {
        kernel::memory_free(mailbox->buffer);
        mailbox->buffer = nullptr;
    }
    if (mailbox->lock) {
        mailbox->lock->methods[0](mailbox->lock, 1);
        mailbox->lock = nullptr;
    }
}

void *__fastcall delete_mailbox(Mailbox *mailbox, void *, unsigned flags) {
    destroy_mailbox(mailbox);
    if (flags & 1)
        kernel::memory_free(mailbox);
    return mailbox;
}
const gui::Destroy mailbox_methods[]{reinterpret_cast<gui::Destroy>(delete_mailbox)};
} // namespace nfs::network::comm
