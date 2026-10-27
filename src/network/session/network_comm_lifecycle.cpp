#include "../transport/comm_connection_runtime.hpp"
#include "nfs/network/transport/network_mailbox.hpp"
#include <cstdlib>
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network::session::pending {
namespace {
Dword options[2]{};
Byte options_initialized{};
Byte updating{};
Dword *drivers[10]{};
Byte drivers_initialized{};
Dword registered_drivers{};
comm::LockObject *connection_lock{};
void *pending_operations{};
} // namespace
Dword *connections{};

static void initialize_options() {
    if (!options_initialized) {
        options[1] = 0;
        options[0] = 1;
        options_initialized = 1;
    }
}

Dword *comm_options() {
    initialize_options();
    return options;
}

static Dword assign_options(const Dword *source) {
    initialize_options();
    if (!source)
        return 0;
    options[0] = source[0];
    options[1] = source[1];
    return 1;
}

static void clear_options() {
    if (options_initialized)
        options_initialized = 0;
}

static Byte initialize_drivers() {
    if (drivers_initialized)
        return 1;
    std::memset(drivers, 0, sizeof(drivers));
    registered_drivers = 0;
    drivers_initialized = 1;
    return 1;
}

Dword *registered_driver(int index) {
    if (!drivers_initialized)
        initialize_drivers();
    return drivers[index];
}

static Byte initialize_connection_registry() {
    if (!connection_lock) {
        auto *lock = static_cast<comm::LockObject *>(comm::allocate_object(36));
        if (lock) {
            comm::initialize_kind(&lock->kind, 5);
            lock->methods = comm::lock_methods;
            std::memset(&lock->lock, 0, 28);
            kernel::lock_init(&lock->lock);
            connection_lock = lock;
            pending_operations = nullptr;
            return 1;
        }
        connection_lock = nullptr;
    }
    pending_operations = nullptr;
    return 1;
}
// Callers consume AL.
static bool comm_initialized() {
    return comm_api != nullptr;
}
// Constructor has no field writes.
static Dword *construct_comm_api(Dword *owner) {
    return owner;
}

bool initialize_comm(const Dword *settings) {
    if (!comm_initialized()) {
        if (!initialize_drivers()) {
            comm::set_error(44);
            return false;
        }
        if (!initialize_connection_registry()) {
            comm::set_error(70);
            return false;
        }
        if (settings)
            assign_options(settings);
        auto *storage = static_cast<Dword *>(gui::application_allocate(1));
        if (!storage) {
            comm_api = nullptr;
            comm::set_error(6);
            return false;
        }
        comm_api = construct_comm_api(storage);
        if (!comm_api) {
            comm::set_error(6);
            return false;
        }
    }
    return true;
}

static void unlink_pending_operation(void *operation) {
    void *previous = at<void *>(operation, 16), *next = at<void *>(operation, 20);
    if (previous)
        at<void *>(previous, 20) = next;
    if (next)
        at<void *>(next, 16) = previous;
}

static Dword shutdown_connection_registry() {
    for (void *operation = pending_operations; operation;) {
        void *next = at<void *>(operation, 20);
        unlink_pending_operation(operation);
        std::free(operation);
        operation = next;
    }
    if (connection_lock) {
        method(connection_lock, 0, Dword(1));
        connection_lock = nullptr;
    }
    return 1;
}
// Only AL is consumed by the saved shutdown caller.
static Byte shutdown_drivers() {
    if (drivers_initialized) {
        for (unsigned index = 0; index < 9; ++index)
            if (drivers[index])
                method(drivers[index], 0, Dword(1));
        drivers_initialized = 0;
    }
    return 1;
}

static Dword destroy_comm_api() {
    for (Dword *connection = connections; connection;) {
        Dword *next = at<Dword *>(connection, 108);
        method(connection, 1, Dword(1));
        connection = next;
    }
    connections = nullptr;
    return 1;
}

Byte shutdown_comm() {
    if (comm_initialized()) {
        shutdown_connection_registry();
        shutdown_drivers();
        if (comm_api) {
            destroy_comm_api();
            gui::application_release(comm_api);
        }
        comm_api = nullptr;
        clear_options();
    }
    return 1;
}

static Dword update_comm_api() {
    if (!comm_initialized()) {
        comm::set_error(2);
        return 0;
    }
    if (!*comm_options()) {
        if (updating)
            _assert("(gCommApiUpdateBusy==false)", "cmn\\comm_api.cpp", 504);
        updating = 1;
        for (int index = 0; index < 9; ++index)
            if (Dword *driver = registered_driver(index))
                method(driver, 3, Dword(0));
        updating = 0;
    }
    return 1;
}

void update_comm() {
    if (!comm_api)
        _assert("gpCommApi", "cmn\\comm_api.cpp", 448);
    update_comm_api();
}
// Only the defined low byte is consumed here.
bool contains_connection(void *set, void *connection, Dword *index) {
    if (connection) {
        Dword *items = at<Dword *>(set, 8);
        for (int item = 0; item < at<int>(set, 12); ++item)
            if (items[item] == reinterpret_cast<Dword>(connection)) {
                if (index)
                    *index = Dword(item);
                return true;
            }
    }
    return false;
}

Dword find_pending_connection(void *driver, void *connection) {
    for (void *set = at<void *>(driver, 20); set; set = at<void *>(set, 24))
        if (contains_connection(set, connection, nullptr))
            return reinterpret_cast<Dword>(set);
    return 0;
}

Dword register_driver(void *driver) {
    if (!drivers_initialized)
        initialize_drivers();
    if (!driver)
        return 0;
    drivers[at<Dword>(driver, 12)] = static_cast<Dword *>(driver);
    return 1;
}

Dword unregister_driver(void *driver) {
    if (!drivers_initialized)
        initialize_drivers();
    if (!driver)
        return 0;
    drivers[at<Dword>(driver, 12)] = nullptr;
    return 1;
}

static Dword connection_registered(void *connection) {
    if (!comm_initialized()) {
        comm::set_error(2);
        return 0;
    }
    for (void *current = connections; current; current = at<void *>(current, 108))
        if (current == connection)
            return 1;
    return 0;
}

static Dword append_connection(void *connection) {
    if (!comm_initialized()) {
        comm::set_error(2);
        return 0;
    }
    if (!connection)
        return 0;
    if (!connection_registered(connection)) {
        for (void *current = connections; current; current = at<void *>(current, 108))
            if (!at<void *>(current, 108)) {
                at<void *>(current, 108) = connection;
                at<void *>(connection, 104) = current;
                return 1;
            }
        connections = static_cast<Dword *>(connection);
    }
    return 1;
}

Dword register_connection(void *connection) {
    if (!comm_api)
        _assert("gpCommApi", "cmn\\comm_api.cpp", 298);
    return append_connection(connection);
}

static Dword remove_connection(void *connection) {
    if (!comm_initialized()) {
        comm::set_error(2);
        return 0;
    }
    if (!connection)
        return 0;
    if (connection == connections)
        connections = at<Dword *>(connection, 108);
    void *previous = at<void *>(connection, 104), *next = at<void *>(connection, 108);
    if (previous)
        at<void *>(previous, 108) = next;
    if (next)
        at<void *>(next, 104) = previous;
    return 1;
}

Dword unregister_connection(void *connection) {
    if (!comm_api)
        _assert("gpCommApi", "cmn\\comm_api.cpp", 311);
    return remove_connection(connection);
}
} // namespace nfs::network::session::pending
