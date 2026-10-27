#include "comm_filter_runtime.hpp"
namespace nfs::network::session::pending {

void *construct_filter(void *owner, Dword kind) {
    comm::initialize_kind(&at<Dword>(owner, 4), 2);
    at<Dword>(owner, 24) = 1;
    at<Dword>(owner, 8) = kind;
    at<void *>(owner, 12) = at<void *>(owner, 16) = at<void *>(owner, 20) = nullptr;
    at<const Dword *>(owner, 0) = filter_methods;
    return owner;
}

void destroy_filter(void *owner) {
    at<const Dword *>(owner, 0) = filter_methods;
    if (void *connection = at<void *>(owner, 12))
        unlink_connection_filter(connection, owner);
}

static void *__fastcall delete_filter(void *owner, void *, Dword flags) {
    destroy_filter(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

Dword __fastcall filter_kind(void *owner, void *) {
    return at<Dword>(owner, 8);
}

Dword __fastcall filter_mode(void *owner, void *) {
    return at<Dword>(owner, 24);
}

Dword __fastcall set_filter_mode(void *owner, void *, Dword value) {
    const Dword previous = at<Dword>(owner, 24);
    at<Dword>(owner, 24) = value;
    return previous;
}

void __fastcall enable_filter(void *owner, void *) {
    method(owner, 3, Dword(1));
}

void __fastcall disable_filter(void *owner, void *) {
    method(owner, 3, Dword(2));
}
// Complete one-returning AL leaf.
Byte __fastcall clear_filter(void *, void *) {
    return 1;
}

void *__fastcall filter_connection(void *owner, void *) {
    return at<void *>(owner, 12);
}

void *__fastcall set_filter_connection(void *owner, void *, void *value) {
    void *previous = at<void *>(owner, 12);
    at<void *>(owner, 12) = value;
    return previous;
}

void *__fastcall filter_previous(void *owner, void *) {
    return at<void *>(owner, 16);
}

void *__fastcall set_filter_previous(void *owner, void *, void *value) {
    void *previous = at<void *>(owner, 16);
    at<void *>(owner, 16) = value;
    return previous;
}

void *__fastcall filter_next(void *owner, void *) {
    return at<void *>(owner, 20);
}

void *__fastcall set_filter_next(void *owner, void *, void *value) {
    at<void *>(owner, 20) = value;
    return at<void *>(owner, 16);
}
// Complete zero-returning header-size leaf.
Dword __fastcall filter_header_bytes(void *, void *) {
    return 0;
}

Dword __fastcall filter_total_header_bytes(void *owner, void *) {
    Dword bytes = 0;
    for (void *filter = owner; filter; filter = method<void *>(filter, 12))
        bytes += method<Dword>(filter, 18);
    if (void *connection = at<void *>(owner, 12))
        if (void *driver = at<void *>(connection, 84))
            bytes += method<Dword>(driver, 6);
    return bytes;
}

Byte __fastcall filter_send(void *owner, void *, comm::Packet *packet) {
    if (!at<void *>(owner, 12))
        return 0;
    if (void *lower = at<void *>(owner, 20))
        return method<Byte>(lower, 14, packet);
    return comm::send_via_driver(at<Dword *>(owner, 12), packet) != 0;
}
// The unbound-connection leaf defines only AL.
Dword __fastcall filter_receive(void *owner, void *, comm::Packet *packet) {
    if (!at<void *>(owner, 12))
        return reinterpret_cast<Dword>(owner) & 4294967040u;
    if (void *upper = at<void *>(owner, 16))
        return method<Dword>(upper, 15, packet);
    return deliver_connection_packet(at<void *>(owner, 12), packet);
}

static Byte __fastcall base_filter_send(void *owner, void *, comm::Packet *packet) {
    return method<Byte>(owner, 16, packet);
}

static Dword __fastcall base_filter_receive(void *owner, void *, comm::Packet *packet) {
    return method<Dword>(owner, 17, packet);
}

static void *__fastcall clone_filter(void *owner, void *) {
    void *storage = comm::allocate_object(28);
    return storage ? construct_filter(storage, at<Dword>(owner, 8)) : nullptr;
}
const Dword filter_methods[]{
    reinterpret_cast<Dword>(delete_filter),       reinterpret_cast<Dword>(filter_kind),
    reinterpret_cast<Dword>(filter_mode),         reinterpret_cast<Dword>(set_filter_mode),
    reinterpret_cast<Dword>(enable_filter),       reinterpret_cast<Dword>(disable_filter),
    reinterpret_cast<Dword>(clear_filter),        reinterpret_cast<Dword>(clone_filter),
    reinterpret_cast<Dword>(filter_connection),   reinterpret_cast<Dword>(set_filter_connection),
    reinterpret_cast<Dword>(filter_previous),     reinterpret_cast<Dword>(set_filter_previous),
    reinterpret_cast<Dword>(filter_next),         reinterpret_cast<Dword>(set_filter_next),
    reinterpret_cast<Dword>(base_filter_send),    reinterpret_cast<Dword>(base_filter_receive),
    reinterpret_cast<Dword>(filter_send),         reinterpret_cast<Dword>(filter_receive),
    reinterpret_cast<Dword>(filter_header_bytes), reinterpret_cast<Dword>(filter_total_header_bytes)};
} // namespace nfs::network::session::pending
