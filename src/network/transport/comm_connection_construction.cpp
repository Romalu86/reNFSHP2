#include "comm_connection_runtime.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::network::session::pending {
comm::LockObject *allocate_comm_lock() {
    auto *lock = static_cast<comm::LockObject *>(comm::allocate_object(36));
    if (lock) {
        comm::initialize_kind(&lock->kind, 5);
        lock->methods = comm::lock_methods;
        std::memset(&lock->lock, 0, 28);
        kernel::lock_init(&lock->lock);
    }
    return lock;
}

Dword reset_connection(void *owner, void *source) {
    if (!source) {
        at<Dword>(owner, 12) = 0;
        at<Dword>(owner, 16) = 0;
        at<Dword>(owner, 20) = 1;
        at<std::uint16_t>(owner, 72) = 0;
        at<void *>(owner, 76) = nullptr;
        at<void *>(owner, 84) = nullptr;
    } else {
        for (unsigned offset : {12u, 16u, 20u, 76u, 84u})
            at<Dword>(owner, offset) = at<Dword>(source, offset);
        at<std::uint16_t>(owner, 72) = at<std::uint16_t>(source, 72);
    }
    for (unsigned offset : {68u, 88u, 104u, 108u})
        at<Dword>(owner, offset) = 0;
    at<void *>(owner, 80) = allocate_comm_lock();
    auto *storage = static_cast<comm::Mailbox *>(comm::allocate_object(36));
    at<comm::Mailbox *>(owner, 92) =
        storage ? comm::construct_mailbox(storage, at<Dword>(owner, 64)) : nullptr;
    at<void *>(owner, 96) = allocate_comm_lock();
    return 1;
}

static void construct_connection(Dword *owner, Dword type, Dword protocol_kind,
                                 const ConnectionOptions *options, Dword *callback) {
    *owner = reinterpret_cast<Dword>(comm_object_methods);
    comm::initialize_kind(owner + 1, 1);
    owner[2] = 1;
    construct_connection_options(reinterpret_cast<ConnectionOptions *>(owner + 6));
    *owner = reinterpret_cast<Dword>(connection_methods);
    if (!callback)
        _assert("pCallback", "cmn\\connection.cpp", 49);
    if (options)
        std::memcpy(owner + 6, options, 44);
    reset_connection(owner, nullptr);
    owner[3] = type;
    owner[4] = protocol_kind;
    owner[5] = 1;
    owner[19] = reinterpret_cast<Dword>(callback);
    owner[17] = 0;
    if (!at<std::uint16_t>(owner, 38))
        at<std::uint16_t>(owner, 38) = 4096;
    if (!at<std::uint16_t>(owner, 42))
        at<std::uint16_t>(owner, 42) = 32;
    if (!at<std::uint16_t>(owner, 44))
        at<std::uint16_t>(owner, 44) = 1024;
    owner[22] = 0;
    if (at<Byte>(owner, 40) && protocol_kind == 1) {
        _assert("!\"Do not use llpacket on a reliable TCP/IP connection connection.\"",
                "cmn\\connection.cpp", 91);
        return;
    }
    if (at<Byte>(owner, 36)) {
        void *storage = comm::allocate_object(152);
        link_connection_filters(owner, storage ? construct_packetizer_filter(storage) : nullptr);
    }
    if (at<Byte>(owner, 40)) {
        void *storage = comm::allocate_object(96);
        auto *settings = const_cast<ConnectionOptions *>(options);
        link_connection_filters(
            owner, storage ? construct_reliable_filter(storage, at<std::uint16_t>(settings, 18),
                                                       at<std::uint16_t>(settings, 20))
                           : nullptr);
    }
    register_connection(owner);
}

Dword *construct_inet_connection(Dword *owner, Dword protocol_kind, const ConnectionOptions *options,
                                Dword *callback) {
    construct_connection(owner, 1, protocol_kind, options, callback);
    *owner = reinterpret_cast<Dword>(inet_connection_methods);
    at<void *>(owner, 84) = options && at<void *>(const_cast<ConnectionOptions *>(options), 8)
                                  ? at<void *>(const_cast<ConnectionOptions *>(options), 8)
                                  : get_inet_driver();
    if (!at<void *>(owner, 84))
        _assert("m_pDriver", "cmn\\connection_inet.cpp", 46);
    std::memset(bytes(owner) + 112, 0, 256);
    at<SOCKET>(owner, 368) = INVALID_SOCKET;
    std::memset(bytes(owner) + 372, 0, 16);
    std::memset(bytes(owner) + 388, 0, 16);
    return owner;
}

void *construct_tcp_connection(void *storage, const ConnectionOptions *options, Dword *callback) {
    auto *owner = construct_inet_connection(static_cast<Dword *>(storage), 1, options, callback);
    *owner = reinterpret_cast<Dword>(tcp_connection_methods);
    return owner;
}

Dword create_inet_socket(void *owner) {
    at<SOCKET>(owner, 368) = create_connection_socket(owner);
    driver_add_connection(at<void *>(owner, 84), owner);
    return 1;
}

void *construct_udp_connection(void *storage, const ConnectionOptions *options, Dword *callback) {
    auto *owner = construct_inet_connection(static_cast<Dword *>(storage), 2, options, callback);
    *owner = reinterpret_cast<Dword>(udp_connection_methods);
    create_inet_socket(owner);
    swap_error(owner, 4);
    return owner;
}

static void copy_connection(Dword *owner, void *source) {
    *owner = reinterpret_cast<Dword>(comm_object_methods);
    comm::initialize_kind(owner + 1, 1);
    owner[2] = 1;
    construct_connection_options(reinterpret_cast<ConnectionOptions *>(owner + 6));
    *owner = reinterpret_cast<Dword>(connection_methods);
    std::memcpy(owner + 6, bytes(source) + 24, 44);
    reset_connection(owner, source);
    for (void *filter = at<void *>(source, 88); filter; filter = method<void *>(filter, 12))
        link_connection_filters(owner, method<void *>(filter, 7));
    register_connection(owner);
}

Dword *copy_inet_connection(Dword *owner, void *source) {
    copy_connection(owner, source);
    *owner = reinterpret_cast<Dword>(inet_connection_methods);
    if (at<Dword>(owner, 16) == 1)
        create_inet_socket(owner);
    return owner;
}

static Dword *copy_udp_connection(Dword *owner, void *source) {
    copy_inet_connection(owner, source);
    *owner = reinterpret_cast<Dword>(udp_connection_methods);
    at<SOCKET>(owner, 368) = at<SOCKET>(source, 368);
    return owner;
}

void *__fastcall clone_inet_connection(void *source, void *) {
    auto *owner = static_cast<Dword *>(comm::allocate_object(404));
    return owner ? copy_inet_connection(owner, source) : nullptr;
}

void *__fastcall clone_udp_connection(void *source, void *) {
    auto *owner = static_cast<Dword *>(comm::allocate_object(404));
    return owner ? copy_udp_connection(owner, source) : nullptr;
}

Dword close_connection(void *owner) {
    if (at<Dword>(owner, 68)) {
        swap_error(static_cast<Dword *>(owner), 5);
        if (void *driver = at<void *>(owner, 84))
            method(driver, 5, owner);
        swap_error(static_cast<Dword *>(owner), 0);
    }
    return 1;
}

void destroy_connection(void *owner) {
    at<const Dword *>(owner, 0) = connection_methods;
    unregister_connection(owner);
    close_connection(owner);
    clear_connection_filters(owner);
    for (unsigned offset : {92u, 96u, 80u})
        if (void *object = at<void *>(owner, offset))
            method(object, 0, Dword(1));
    connection_event(owner, nullptr, 1002, 0, 0, owner);
    if (at<Dword>(owner, 8) != 1)
        _assert("m_refcount == 1", "cmn\\refcount.cpp", 39);
}

void destroy_inet_connection(void *owner) {
    at<const Dword *>(owner, 0) = inet_connection_methods;
    if (void *driver = at<void *>(owner, 84))
        driver_remove_connection(driver, owner);
    destroy_connection(owner);
}

static void destroy_tcp_connection(void *owner) {
    at<const Dword *>(owner, 0) = tcp_connection_methods;
    close_connection(owner);
    destroy_inet_connection(owner);
}

static void destroy_udp_connection(void *owner) {
    at<const Dword *>(owner, 0) = udp_connection_methods;
    close_connection(owner);
    destroy_inet_connection(owner);
}

void *__fastcall delete_inet_connection(void *owner, void *, Dword flags) {
    destroy_inet_connection(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void *__fastcall delete_tcp_connection(void *owner, void *, Dword flags) {
    destroy_tcp_connection(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void *__fastcall delete_udp_connection(void *owner, void *, Dword flags) {
    destroy_udp_connection(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

static void *__fastcall delete_connection(void *owner, void *, Dword flags) {
    destroy_connection(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

static void *__fastcall clone_connection(void *source, void *) {
    auto *owner = static_cast<Dword *>(comm::allocate_object(112));
    if (owner)
        copy_connection(owner, source);
    return owner;
}
// Complete base-class zero-returning method.
static Dword __fastcall base_address_text(void *, void *, Dword) {
    return 0;
}
// Complete base-class zero-returning method.
static Dword __fastcall base_address_number(void *, void *, Dword) {
    return 0;
}
const Dword connection_methods[]{
    reinterpret_cast<Dword>(connection_event),  reinterpret_cast<Dword>(delete_connection),
    reinterpret_cast<Dword>(base_address_text), reinterpret_cast<Dword>(base_address_number),
    reinterpret_cast<Dword>(clone_connection),  reinterpret_cast<Dword>(close_connection_method),
    reinterpret_cast<Dword>(update_connection), reinterpret_cast<Dword>(send_connection)};
const Dword comm_object_methods[]{reinterpret_cast<Dword>(assertions::pure_virtual_call)};
} // namespace nfs::network::session::pending
