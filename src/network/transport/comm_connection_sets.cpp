#include "comm_connection_runtime.hpp"
namespace nfs::network::session::pending {

void *construct_connection_set(void *owner) {
    comm::initialize_kind(&at<Dword>(owner, 4), 10);
    at<const Dword *>(owner, 0) = connection_set_methods;
    at<Dword>(owner, 12) = 64;
    at<void *>(owner, 8) = comm::allocate_named("CONNECTION_POOL", 256, 0);
    std::memset(at<void *>(owner, 8), 0, 256);
    at<void *>(owner, 24) = nullptr;
    at<void *>(owner, 20) = nullptr;
    at<void *>(owner, 16) = allocate_comm_lock();
    return owner;
}

static void destroy_connection_set(void *owner) {
    at<const Dword *>(owner, 0) = connection_set_methods;
    if (void *pool = at<void *>(owner, 8)) {
        kernel::memory_free(pool);
        at<void *>(owner, 8) = nullptr;
    }
    if (void *lock = at<void *>(owner, 16))
        method(lock, 0, Dword(1));
}

static void *__fastcall delete_connection_set(void *owner, void *, Dword flags) {
    destroy_connection_set(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void link_connection_set(void *owner, void *next) {
    if (at<void *>(owner, 24) != next && next) {
        at<void *>(owner, 24) = next;
        if (at<void *>(next, 20) != owner && owner) {
            at<void *>(next, 20) = owner;
            link_connection_set(owner, next);
        }
    }
}

static void link_previous_connection_set(void *owner, void *previous) {
    if (at<void *>(owner, 20) != previous && previous) {
        at<void *>(owner, 20) = previous;
        link_connection_set(previous, owner);
    }
}

bool add_connection_to_set(void *owner, void *connection) {
    if (connection) {
        if (contains_connection(owner, connection, nullptr))
            return true;
        auto **items = at<void **>(owner, 8);
        for (int i = 0; i < at<int>(owner, 12); ++i)
            if (!items[i]) {
                items[i] = connection;
                return true;
            }
    }
    return false;
}

bool remove_connection_from_set(void *owner, void *connection) {
    if (!connection)
        return false;
    enter_comm_lock(at<void *>(owner, 16));
    Dword index = reinterpret_cast<Dword>(connection);
    const bool found = contains_connection(owner, connection, &index);
    if (found)
        at<void **>(owner, 8)[index] = nullptr;
    leave_comm_lock(at<void *>(owner, 16));
    return found;
}

static void *connection_set_item(void *owner, int index) {
    void *connection = nullptr;
    enter_comm_lock(at<void *>(owner, 16));
    if (index >= 0 && index < at<int>(owner, 12))
        connection = at<void **>(owner, 8)[index];
    leave_comm_lock(at<void *>(owner, 16));
    return connection;
}

bool driver_add_connection(void *owner, void *connection) {
    if (!connection) {
        comm::set_error(3);
        return false;
    }
    if (!find_pending_connection(owner, connection)) {
        void *previous = at<void *>(owner, 20);
        for (void *set = previous; set; previous = set, set = at<void *>(set, 24))
            if (add_connection_to_set(set, connection))
                return true;
        void *storage = comm::allocate_object(28);
        void *set = storage ? construct_connection_set(storage) : nullptr;
        link_previous_connection_set(set, previous);
        return add_connection_to_set(set, connection);
    }
    return true;
}

Byte driver_remove_connection(void *owner, void *connection) {
    if (!connection)
        return 0;
    for (void *set = at<void *>(owner, 20); set; set = at<void *>(set, 24))
        if (remove_connection_from_set(set, connection))
            return 1;
    return 0;
}

void __fastcall update_connection_set(void *, void *, void *set) {
    for (int index = 0; index < at<int>(set, 12); ++index) {
        void *connection = connection_set_item(set, index);
        if (!connection)
            continue;
        switch (at<Dword>(connection, 68)) {
        case 0:
            break;
        case 1:
            if (void *callback = at<void *>(connection, 76))
                method(callback, 0, Dword(5), reinterpret_cast<Dword>(connection), Dword(4),
                       connection);
            break;
        case 2:
            method(connection, 1, Dword(1));
            break;
        case 3:
        case 4:
        case 5:
            method(connection, 6);
            break;
        default:
            _assert("0", "cmn\\driver.cpp", 526);
            break;
        }
    }
}
const Dword connection_set_methods[]{reinterpret_cast<Dword>(delete_connection_set)};
} // namespace nfs::network::session::pending
