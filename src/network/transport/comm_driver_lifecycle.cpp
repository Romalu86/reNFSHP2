#include "comm_connection_runtime.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::network::session::pending {
extern const Dword driver_methods[];

void construct_driver(void *owner, Dword kind) {
    comm::initialize_kind(&at<Dword>(owner, 4), 3);
    at<Dword>(owner, 8) = 1;
    at<Dword>(owner, 12) = kind;
    for (unsigned offset : {16u, 20u, 24u, 28u})
        at<Dword>(owner, offset) = 0;
    at<const Dword *>(owner, 0) = driver_methods;
    at<void *>(owner, 28) = allocate_comm_lock();
    void *set = comm::allocate_object(28);
    at<void *>(owner, 20) = set ? construct_connection_set(set) : nullptr;
}

Dword __fastcall initialize_driver(void *owner, void *) {
    register_driver(owner);
    at<Dword>(owner, 16) = 2;
    return 1;
}

Dword __fastcall stop_driver(void *owner, void *) {
    if (at<Dword>(owner, 16)) {
        void *worker = at<void *>(owner, 24);
        at<Dword>(owner, 16) = 3;
        if (worker) {
            while (at<volatile Dword>(worker, 20) != 3) {
                at<volatile Dword>(worker, 20) = 2;
                kernel::thread_yield(10);
                worker = at<void *>(owner, 24);
            }
            if (at<void *>(owner, 24))
                method(at<void *>(owner, 24), 0, Dword(1));
            at<void *>(owner, 24) = nullptr;
        }
        at<Dword>(owner, 16) = 0;
        unregister_driver(owner);
    }
    return 1;
}

void __fastcall update_driver(void *owner, void *, Dword direction) {
    enter_comm_lock(at<void *>(owner, 28));
    for (void *set = at<void *>(owner, 20); set && at<Dword>(owner, 16) == 2;
         set = at<void *>(set, 24)) {
        if (!direction || direction == 1)
            method(owner, 9, set);
        if (!direction || direction == 2)
            method(owner, 10, set);
    }
    leave_comm_lock(at<void *>(owner, 28));
}

void destroy_driver(void *owner) {
    at<const Dword *>(owner, 0) = driver_methods;
    stop_driver(owner, nullptr);
    for (void *connection = connections; connection; connection = at<void *>(connection, 108))
        if (at<void *>(connection, 84) == owner)
            method(connection, 0, Dword(1003), reinterpret_cast<Dword>(owner), Dword(4), owner);
    void *set = at<void *>(owner, 20);
    while (set) {
        void *next = at<void *>(set, 24);
        method(set, 0, Dword(1));
        set = next;
    }
    if (void *lock = at<void *>(owner, 28))
        method(lock, 0, Dword(1));
    if (at<Dword>(owner, 8) != 1)
        _assert("m_refcount == 1", "cmn\\refcount.cpp", 39);
}
// Full zero-returning leaf.
Dword __fastcall driver_option_leaf(void *, void *) {
    return 0;
}
// Complete one-returning method.
static Byte __fastcall base_can_send(void *, void *, void *) {
    return 1;
}

static void *__fastcall delete_driver(void *owner, void *, Dword flags) {
    destroy_driver(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void *get_inet_driver() {
    if (!comm_api) {
        comm::set_error(2);
        return nullptr;
    }
    if (void *existing = registered_driver(1))
        return existing;
    void *owner = comm::allocate_object(292);
    if (!owner)
        return nullptr;
    construct_driver(owner, 1);
    at<const Dword *>(owner, 0) = inet_driver_methods;
    at<void *>(owner, 288) = allocate_comm_lock();
    WSADATA winsock;
    if (WSAStartup(514, &winsock)) {
        if (!WSAGetLastError())
            _assert("errCode != 0", "cmn\\driver_inet.cpp", 121);
        comm::set_error(53);
    }
    start_driver_worker(owner, update_driver_worker, owner, 8192, 2);
    initialize_driver(owner, nullptr);
    return owner;
}

Dword __fastcall cleanup_inet_driver(void *, void *) {
    if (WSACleanup())
        comm::set_error(54);
    return 1;
}

static void destroy_inet_driver(void *owner) {
    at<const Dword *>(owner, 0) = inet_driver_methods;
    stop_driver(owner, nullptr);
    cleanup_inet_driver(owner, nullptr);
    if (void *lock = at<void *>(owner, 288))
        method(lock, 0, Dword(1));
    destroy_driver(owner);
}

void *__fastcall delete_inet_driver(void *owner, void *, Dword flags) {
    destroy_inet_driver(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}
const Dword inet_driver_methods[]{
    reinterpret_cast<Dword>(delete_inet_driver),    reinterpret_cast<Dword>(initialize_driver),
    reinterpret_cast<Dword>(stop_driver),           reinterpret_cast<Dword>(update_driver),
    reinterpret_cast<Dword>(open_inet_connection),  reinterpret_cast<Dword>(close_inet_connection),
    reinterpret_cast<Dword>(driver_option_leaf),    reinterpret_cast<Dword>(can_send_inet),
    reinterpret_cast<Dword>(send_inet_packet),      reinterpret_cast<Dword>(poll_inet_set),
    reinterpret_cast<Dword>(update_connection_set), reinterpret_cast<Dword>(cleanup_inet_driver)};
const Dword driver_methods[]{reinterpret_cast<Dword>(delete_driver),
                            reinterpret_cast<Dword>(initialize_driver),
                            reinterpret_cast<Dword>(stop_driver),
                            reinterpret_cast<Dword>(update_driver),
                            reinterpret_cast<Dword>(assertions::pure_virtual_call),
                            reinterpret_cast<Dword>(assertions::pure_virtual_call),
                            reinterpret_cast<Dword>(driver_option_leaf),
                            reinterpret_cast<Dword>(base_can_send),
                            reinterpret_cast<Dword>(assertions::pure_virtual_call),
                            reinterpret_cast<Dword>(assertions::pure_virtual_call),
                            reinterpret_cast<Dword>(update_connection_set)};
} // namespace nfs::network::session::pending
