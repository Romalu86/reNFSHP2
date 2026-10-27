#include "network_lifecycle_runtime.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::network::session {

ConnectionOptions *construct_connection_options(ConnectionOptions *options) {
    for (unsigned offset : {0u, 4u, 8u, 28u, 32u})
        at<Dword>(options, offset) = 0;
    at<Byte>(options, 36) = 1;
    at<Dword>(options, 40) = 20480;
    at<Byte>(options, 24) = 0;
    at<Byte>(options, 12) = 1;
    at<Byte>(options, 13) = 0;
    at<std::uint16_t>(options, 14) = 4096;
    at<Byte>(options, 16) = 0;
    at<std::uint16_t>(options, 18) = 32;
    at<std::uint16_t>(options, 20) = 1024;
    return options;
}

void create_transport_connections(void *owner) {
    ConnectionOptions options;
    construct_connection_options(&options);
    at<Dword>(&options, 0) = 0;
    at<Byte>(&options, 12) = 1;
    at<Byte>(&options, 13) = 0;
    at<Byte>(&options, 16) = 0;
    const char *labels[]{"REALCOMM::CONNECTION_TCP", "REALCOMM::CONNECTION_UDP",
                         "REALCOMM::CONNECTION_UDP_IN"};
    for (unsigned index = 0; index < 3; ++index) {
        if (index == 1)
            at<Byte>(&options, 12) = 0;
        void *&connection = at<void *>(owner, 40 + index * 4);
        if (connection)
            continue;
        void *storage = lifecycle::allocate(404, labels[index], lifecycle::level_pool);
        try {
            connection =
                storage
                    ? (index == 0 ? pending::construct_tcp_connection(storage, &options, transport)
                                  : pending::construct_udp_connection(storage, &options, transport))
                    : nullptr;
        } catch (...) {
            gui::application_release(storage);
            throw;
        }
    }
}

void release_transport_connections(void *owner) {
    for (unsigned offset : {40u, 44u, 48u}) {
        void *connection = at<void *>(owner, offset);
        if (!connection)
            continue;
        void *manager = reinterpret_cast<void *>(pending_packets(connection));
        destroy_packet(static_cast<Dword *>(connection));
        while (pending::find_pending_connection(manager, connection)) {
            pending::update_comm();
            kernel::thread_yield(1);
        }
        at<void *>(owner, offset) = nullptr;
    }
}

void reset_transport_errors(void *owner) {
    for (unsigned offset : {40u, 44u, 48u}) {
        void *connection = at<void *>(owner, offset);
        if (connection && network::read_error(connection)) {
            method(connection, 5);
            if (offset == 40)
                std::memset(bytes(connection) + 388, 0, 16);
        }
    }
}

void __cdecl transport_worker(Dword argument) {
    auto *owner = reinterpret_cast<TransportLayout *>(argument);
    kernel::signal_wait(&owner->wake);
    while (!owner->stop) {
        pending::update_comm();
        kernel::thread_yield(1);
        if (owner->stop)
            return;
        kernel::signal_wait(&owner->wake);
    }
}

void listen_transport(void *owner) {
    if (!method<Byte>(at<void *>(owner, 40), 8, Dword(0), at<std::uint16_t>(owner, 36)))
        at<Dword>(owner, 4) = 0;
    if (!method<Byte>(at<void *>(owner, 48), 8, Dword(0), at<std::uint16_t>(owner, 38)))
        at<Dword>(owner, 4) = 0;
}

bool connect_transport(void *owner, Dword address, std::uint16_t port, Dword flags) {
    if (port)
        at<std::uint16_t>(owner, 36) = port;
    void *connection = at<void *>(owner, 40);
    if (!connection)
        return false;
    auto *wake = reinterpret_cast<kernel::Signal *>(bytes(owner) + 72);
    if (read_error(connection)) {
        method(connection, 5);
        kernel::signal_set(wake);
        kernel::thread_yield(10);
    }
    connection = at<void *>(owner, 40);
    const bool connected =
        method<Byte>(connection, 9, address, at<std::uint16_t>(owner, 36), flags) != 0;
    if (!connected) {
        swap_error(connection, 1);
        method(connection, 5);
        kernel::signal_set(wake);
        kernel::thread_yield(10);
    }
    kernel::signal_set(wake);
    return connected;
}
} // namespace nfs::network::session
namespace nfs::services::pending {

void prepare_network_transport(void *object) {
    using namespace network::session;
    auto *owner = static_cast<TransportLayout *>(object);
    if (owner->active)
        return;
    Dword *options = network::session::pending::comm_options();
    options[0] = 1;
    if (!network::session::pending::initialize_comm(options)) {
        owner->active = 0;
        return;
    }
    owner->active = 1;
    create_transport_connections(owner);
    kernel::signal_init(&owner->wake);
    owner->stop = 0;
    kernel::thread_start(&owner->worker, transport_worker, reinterpret_cast<Dword>(owner), 0, 0, 0);
    kernel::signal_set(&owner->wake);
}

void stop_network_transport(void *object) {
    using namespace network::session;
    auto *owner = static_cast<TransportLayout *>(object);
    if (!owner->active)
        return;
    owner->stop = 1;
    kernel::signal_set(&owner->wake);

    const Dword start = kernel::timer_count();
    while (!kernel::thread_finished(&owner->worker))
        if (!(static_cast<std::int32_t>(start + 4000u) <
              static_cast<std::int32_t>(kernel::timer_count())))
            break;
    kernel::signal_destroy(&owner->wake);
    reset_transport_errors(owner);
    release_transport_connections(owner);
    network::session::pending::shutdown_comm();
    owner->active = 0;
}
} // namespace nfs::services::pending
