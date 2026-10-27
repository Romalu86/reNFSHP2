#include "comm_filter_runtime.hpp"
namespace nfs::network::session::pending {
struct ReliableHeader {
    Dword sequence, session;
    Byte flags, padding[3];
};
static_assert(sizeof(ReliableHeader) == 12);
void send_reliable_handshake(void *, Dword, Byte);
void send_reliable_ping(void *, Byte);
Dword send_reliable_batch(void *, int);
Dword accept_reliable_handshake(void *, const ReliableHeader *);
Dword accept_reliable_ping(void *, const ReliableHeader *);
Dword accept_reliable_data(void *, const ReliableHeader *, comm::Packet *);

void *construct_reliable_filter(void *owner, Dword capacity, std::uint16_t packet_size) {
    construct_filter(owner, 1);
    at<const Dword *>(owner, 0) = reliable_filter_methods;
    for (unsigned offset : {48u, 52u, 56u, 84u})
        at<Dword>(owner, offset) = 0;
    const Dword now = comm_milliseconds();
    at<Dword>(owner, 60) = at<Dword>(owner, 40) = now;
    at<Dword>(owner, 64) = 0;
    at<Dword>(owner, 36) = 2000;
    at<Byte>(owner, 44) = 0;
    at<Dword>(owner, 32) = 2000;
    at<Dword>(owner, 28) = 0;
    at<Dword>(owner, 76) = capacity ? capacity : 1;
    at<std::uint16_t>(owner, 80) = packet_size < 22 ? 22 : packet_size;
    for (unsigned offset : {88u, 92u}) {
        auto *pool = static_cast<ReliablePool *>(comm::allocate_object(40));
        at<ReliablePool *>(owner, offset) =
            pool ? construct_reliable_pool(pool, at<Dword>(owner, 76),
                                           at<std::uint16_t>(owner, 80))
                 : nullptr;
    }
    at<void *>(owner, 68) = allocate_comm_lock();
    at<void *>(owner, 72) = allocate_comm_lock();
    return owner;
}

static void destroy_reliable_filter(void *owner) {
    at<const Dword *>(owner, 0) = reliable_filter_methods;
    for (unsigned offset : {88u, 92u, 68u, 72u})
        if (void *object = at<void *>(owner, offset))
            method(object, 0, Dword(1));
    destroy_filter(owner);
}

static void *__fastcall delete_reliable_filter(void *owner, void *, Dword flags) {
    destroy_reliable_filter(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

static void *__fastcall clone_reliable_filter(void *owner, void *) {
    void *storage = comm::allocate_object(96);
    return storage ? construct_reliable_filter(storage, at<Dword>(owner, 76),
                                               at<std::uint16_t>(owner, 80))
                   : nullptr;
}
// Complete header-size leaf.
static Dword __fastcall reliable_header_bytes(void *, void *) {
    return 18;
}

static Byte __fastcall send_reliable(void *owner, void *, comm::Packet *packet) {
    if (at<Dword>(owner, 24) == 2)
        return method<Byte>(owner, 16, packet);
    Byte result = 1;
    enter_comm_lock(at<void *>(owner, 68));
    if (packet) {
        if (!enqueue_reliable_packet(at<ReliablePool *>(owner, 88), packet)) {
            void *connection = at<void *>(owner, 12);
            method(connection, 0, Dword(9), Dword(27), at<Dword>(packet->endpoint, 0), connection);
            leave_comm_lock(at<void *>(owner, 68));
            return 0;
        }
        comm::set_status(packet, 5);
    }
    const Dword now = comm_milliseconds(), previous = at<Dword>(owner, 60);
    at<Dword>(owner, 60) = now;
    const Dword elapsed = now - previous;
    if (!at<Dword>(owner, 48)) {
        if (!at<Dword>(at<void *>(owner, 12), 20))
            result = 0;
        else {
            at<Dword>(owner, 52) -= elapsed;
            if (at<int>(owner, 52) < 1) {
                if (!at<Dword>(owner, 84))
                    at<Dword>(owner, 84) = comm_timestamp();
                send_reliable_handshake(owner, at<ReliablePool *>(owner, 88)->read, 0);
                const Dword delay = Dword(240) << (at<Dword>(owner, 56) & 31);
                at<Dword>(owner, 52) = delay;
                if (int(delay) > 64000) {
                    void *connection = at<void *>(owner, 12);
                    method(connection, 0, Dword(4), reinterpret_cast<Dword>(connection), Dword(4),
                           connection);
                    result = 0;
                    goto send_complete;
                }
                ++at<Dword>(owner, 56);
            }
            if (packet)
                comm::set_status(packet, 3);
        }
    } else if (at<Dword>(owner, 48) == 1) {
        auto *pool = at<ReliablePool *>(owner, 88);
        ReliableEntry *entry = pool->entries + pool->read % pool->capacity;
        result = entry ? Byte(send_reliable_batch(owner, int(elapsed)))
                       : method<Byte>(owner, 16, static_cast<comm::Packet *>(nullptr));
    } else
        _assert("0", "cmn\\module_llpacket.cpp", 249);
send_complete:
    leave_comm_lock(at<void *>(owner, 68));
    if (!packet) {
        void *connection = method<void *>(owner, 8);
        if (at<Dword>(connection, 16) == 2 && at<Dword>(owner, 48) == 1) {
            const Dword ping_now = comm_milliseconds();
            if (!at<Byte>(owner, 44)) {
                at<Dword>(owner, 32) -= ping_now - at<Dword>(owner, 40);
                if (at<int>(owner, 32) < 1) {
                    at<Dword>(owner, 36) = 2000;
                    at<Byte>(owner, 44) = 1;
                    send_reliable_ping(owner, 0);
                }
            } else
                at<Dword>(owner, 36) -= ping_now - at<Dword>(owner, 40);
            at<Dword>(owner, 40) = ping_now;
            if (at<int>(owner, 36) < 1 && at<Byte>(owner, 44)) {
                at<Byte>(owner, 44) = 0;
                if (at<int>(owner, 28) > 2) {
                    at<Dword>(owner, 28) = 0;
                    connection = at<void *>(owner, 12);
                    method(connection, 0, Dword(4), reinterpret_cast<Dword>(connection), Dword(4),
                           connection);
                    return method<Byte>(owner, 16, static_cast<comm::Packet *>(nullptr));
                }
                at<Dword>(owner, 32) = 0;
                ++at<Dword>(owner, 28);
            }
        }
        result = method<Byte>(owner, 16, static_cast<comm::Packet *>(nullptr));
    }
    return result;
}

static Byte __fastcall receive_reliable(void *owner, void *, comm::Packet *packet) {
    if (at<Dword>(owner, 24) == 2 || !packet)
        return method<Byte>(owner, 17, packet);
    enter_comm_lock(at<void *>(owner, 72));
    ReliableHeader header;
    comm::read_packet(packet, &header, 12);
    decode_comm_header(&header, "I2");
    Dword result;
    if (header.flags & 8)
        result = accept_reliable_handshake(owner, &header);
    else if ((header.flags & 240) == 16)
        result = accept_reliable_ping(owner, &header);
    else
        result = accept_reliable_data(owner, &header, packet);
    leave_comm_lock(at<void *>(owner, 72));
    return Byte(result);
}
const Dword reliable_filter_methods[]{reinterpret_cast<Dword>(delete_reliable_filter),
                                     reinterpret_cast<Dword>(filter_kind),
                                     reinterpret_cast<Dword>(filter_mode),
                                     reinterpret_cast<Dword>(set_filter_mode),
                                     reinterpret_cast<Dword>(enable_filter),
                                     reinterpret_cast<Dword>(disable_filter),
                                     reinterpret_cast<Dword>(clear_filter),
                                     reinterpret_cast<Dword>(clone_reliable_filter),
                                     reinterpret_cast<Dword>(filter_connection),
                                     reinterpret_cast<Dword>(set_filter_connection),
                                     reinterpret_cast<Dword>(filter_previous),
                                     reinterpret_cast<Dword>(set_filter_previous),
                                     reinterpret_cast<Dword>(filter_next),
                                     reinterpret_cast<Dword>(set_filter_next),
                                     reinterpret_cast<Dword>(send_reliable),
                                     reinterpret_cast<Dword>(receive_reliable),
                                     reinterpret_cast<Dword>(filter_send),
                                     reinterpret_cast<Dword>(filter_receive),
                                     reinterpret_cast<Dword>(reliable_header_bytes),
                                     reinterpret_cast<Dword>(filter_total_header_bytes)};
} // namespace nfs::network::session::pending
