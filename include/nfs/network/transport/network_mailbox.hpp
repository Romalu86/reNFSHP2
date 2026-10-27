#pragma once
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::network::comm {
struct LockObject {
    const gui::Destroy *methods;
    Dword kind;
    kernel::Lock lock;
};
struct Entry {
    const gui::Destroy *methods;
    Dword span, bytes, identifier;
    std::uint8_t *endpoint;
    std::uint8_t active, padding[3];
    Entry *next;
};
struct Mailbox {
    const gui::Destroy *methods;
    Dword kind;
    LockObject *lock;
    std::uint8_t *buffer;
    Dword capacity, allocated_bytes;
    Entry *head, *tail;
    Dword count;
};
static_assert(sizeof(LockObject) == 36 && offsetof(LockObject, lock) == 8 &&
              sizeof(Entry) == 28 && sizeof(Mailbox) == 36);
extern const gui::Destroy lock_methods[], entry_methods[], mailbox_methods[];
extern Dword last_error, sequence;
void initialize_kind(Dword *, Dword);
void *allocate_named(const char *, Dword, Dword);
void *allocate_object(Dword);
void fill(void *, Dword, Dword);
void set_error(Dword);
void initialize_entry(Entry *, Dword, const void *, Dword, const char *, std::uint16_t);
Dword entry_bytes(Dword, const char *);
void *entry_data(Entry *);
const char *entry_address(Entry *);
Dword entry_port(Entry *);
Entry *entry_next(Entry *);
void destroy_entry(Entry *);
Mailbox *construct_mailbox(Mailbox *, Dword);
void destroy_mailbox(Mailbox *);
Dword available(Mailbox *, std::uint8_t **);
Entry *append(Mailbox *, Dword, const void *, Dword, const char *, std::uint16_t);
Dword pop(Mailbox *);
Dword clear(Mailbox *);
Dword send(Dword *, const void *, int, const char *, std::uint16_t);
} // namespace nfs::network::comm
