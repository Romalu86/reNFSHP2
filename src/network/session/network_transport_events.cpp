#include "network_lifecycle_runtime.hpp"
#include "nfs/core/services/service_dependencies.hpp"
namespace nfs::network::session {

static Byte encrypted_packet(const Byte *packet) {
    return *packet >> 7;
}

Dword __fastcall transport_event(void *owner, void *, Dword event, void *data, int size,
                                Dword *connection) {
    switch (event) {
    case 1: {
        if (!data || size <= 0)
            break;
        void *receiver = at<void *>(owner, 56);
        if (receiver) {
            auto *packet = static_cast<Byte *>(data);
            if (protocol(connection) == 1 &&
                (encrypted_packet(packet) ||
                 (packet[0] != 16 && packet[0] != 18 && packet[0] != 8))) {
                Dword decoded_size;
                if (Byte *decoded = cipher::decode_locked(reinterpret_cast<Dword>(connection),
                                                          packet, Dword(size), &decoded_size)) {
                    method(receiver, 0, connection, decoded, decoded_size);
                    cipher::release_locked(decoded);
                }
            } else
                method(receiver, 0, connection, packet, Dword(size));
        }
        break;
    }
    case 2:
        if (data) {
            if (protocol(data) == 1)
                cipher::register_connection(reinterpret_cast<lifecycle::System *>(system(12)),
                                            reinterpret_cast<Dword>(data));
            if (at<void *>(owner, 56))
                method(at<void *>(owner, 56), 2, data);
        }
        break;
    case 3:
        if (connection) {
            if (protocol(connection) == 1)
                cipher::register_connection(reinterpret_cast<lifecycle::System *>(system(12)),
                                            reinterpret_cast<Dword>(connection));
            if (at<void *>(owner, 56))
                method(at<void *>(owner, 56), 1, connection);
        }
        break;
    case 4:
    case 5:
        if (data && at<void *>(owner, 56))
            method(at<void *>(owner, 56), 3, data);
        break;
    case 6: {
        const Dword offset = protocol(connection) == 1 ? 24 : 28;
        at<Dword>(owner, offset) = transport_time();
        break;
    }
    }
    return 0;
}
namespace pending {

void transport_statistics(Byte opcode, Dword size, Byte flags) {
    using namespace services::dependency;
    ++combined_filter.bytes;
    combined_filter.recent += size;
    auto &protocol_bucket = (flags & 4) ? udp_filter : tcp_filter;
    ++protocol_bucket.bytes;
    protocol_bucket.recent += size;
    if (opcode > 53)
        opcode = 0;
    auto *bucket = reinterpret_cast<Byte *>(message_filters) + Dword(opcode) * 516;
    ++at<Dword>(bucket, 508);
    at<Dword>(bucket, 512) += size;
}
} // namespace pending
} // namespace nfs::network::session
namespace nfs::services::pending {

const Dword transport_methods[]{reinterpret_cast<Dword>(network::session::transport_event)};

const Dword message_methods[]{reinterpret_cast<Dword>(network::session::receive_transport_packet),
                             reinterpret_cast<Dword>(network::session::connection_opened),
                             reinterpret_cast<Dword>(network::session::connection_accepted),
                             reinterpret_cast<Dword>(network::session::connection_closed)};
} // namespace nfs::services::pending
