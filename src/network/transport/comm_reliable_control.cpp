#include "comm_filter_runtime.hpp"
namespace nfs::network::session::pending {
struct ReliableHeader {
    Dword sequence, session;
    Byte flags, padding[3];
};

void send_reliable_handshake(void *owner, Dword sequence, Byte reply) {
    ReliableHeader header{};
    header.sequence = sequence;
    header.flags = Byte(8 | ((reply << 2) & 4));
    header.session = at<Dword>(owner, 84);
    encode_comm_header(&header, "I2");
    auto *packet = static_cast<comm::Packet *>(comm::allocate_object(320));
    if (packet)
        comm::construct_packet(packet, nullptr, int(method<Dword>(owner, 19)), 12);
    comm::write_packet(packet, &header, 12);
    method(owner, 16, packet);
}

void send_reliable_ping(void *owner, Byte reply) {
    ReliableHeader header{};
    header.flags = Byte(16 | ((reply << 2) & 4));
    encode_comm_header(&header, "I2");
    auto *packet = static_cast<comm::Packet *>(comm::allocate_object(320));
    if (packet)
        comm::construct_packet(packet, nullptr, int(method<Dword>(owner, 19)), 12);
    comm::write_packet(packet, &header, 12);
    method(owner, 16, packet);
}

Dword accept_reliable_handshake(void *owner, const ReliableHeader *header) {
    at<ReliablePool *>(owner, 92)->read = header->sequence;
    if (!(header->flags & 4)) {
        if (at<Dword>(owner, 84) && at<Dword>(owner, 48) == 1 &&
            header->session != at<Dword>(owner, 84)) {
            at<Dword>(owner, 48) = 0;
            void *connection = at<void *>(owner, 12);
            method(connection, 0, Dword(4), reinterpret_cast<Dword>(connection), Dword(4), connection);
            auto *pool = at<ReliablePool *>(owner, 88);
            reset_reliable_pool(pool, pool->epoch, 0);
        }
        if (at<Dword>(owner, 48))
            return 1;
        at<Dword>(owner, 84) = header->session;
        send_reliable_handshake(owner, header->sequence, 1);
    }
    if (!at<Dword>(owner, 48)) {
        at<Dword>(owner, 48) = 1;
        void *connection = at<void *>(owner, 12);
        method(connection, 0, Dword(3), reinterpret_cast<Dword>(connection), Dword(4), connection);
    }
    return 1;
}

Dword accept_reliable_ping(void *owner, const ReliableHeader *header) {
    if (!header)
        _assert("pHeader", "cmn\\module_llpacket.cpp", 700);
    if (header->flags & 4) {
        at<Byte>(owner, 44) = 0;
        at<Dword>(owner, 32) = at<Dword>(owner, 36) = 2000;
        return 1793;
    }
    send_reliable_ping(owner, 1);
    return 1;
}

Dword accept_reliable_data(void *owner, const ReliableHeader *header, comm::Packet *packet) {
    if (!packet || header->session != at<Dword>(owner, 84))
        return 0;
    if (header->flags & 4) {
        acknowledge_reliable_packets(at<ReliablePool *>(owner, 88), header->sequence);
        return 1;
    }
    auto *pool = at<ReliablePool *>(owner, 92);
    const Dword first = pool->read;
    if (!(header->flags & 3))
        return 0;
    int accepted = 0;
    Dword identifier = header->sequence;
    for (Dword i = 0; i < (header->flags & 3); ++i, ++identifier) {
        std::uint16_t count;
        comm::read_packet(packet, &count, 2);
        decode_comm_header(&count, "S1");
        if (first <= identifier) {
            const void *data = comm::packet_data(packet);
            store_reliable_packet(at<ReliablePool *>(owner, 92), identifier, data, count);
            ++accepted;
        }
        comm::read_packet(packet, nullptr, count);
    }
    if (!accepted)
        return 0;
    pool = at<ReliablePool *>(owner, 92);
    ReliableEntry *head = pool->entries + pool->read % pool->capacity;
    ReliableEntry *last = nullptr;
    Dword acknowledged = first;
    if (head) {
        ReliableEntry *entry = head;
        while (!entry->available) {
            acknowledged = entry->identifier;
            ReliableEntry *next = entry->next;
            last = entry;
            if (next == head || !next)
                break;
            entry = next;
        }
    }
    if (!head || acknowledged >= first) {
        ReliableHeader reply = *header;
        reply.sequence = acknowledged;
        reply.flags |= 4;
        encode_comm_header(&reply, "I2");
        auto *response = static_cast<comm::Packet *>(comm::allocate_object(320));
        if (response)
            comm::construct_packet(response, nullptr, int(method<Dword>(owner, 19)), 12);
        comm::write_packet(response, &reply, 12);
        method(owner, 16, response);
        if (response)
            response->methods[0](response, 1);
        at<ReliablePool *>(owner, 92)->read = acknowledged + 1;
    }
    for (ReliableEntry *entry = head; entry;) {
        method(owner, 17, &entry->packet);
        clear_reliable_entry(entry);
        if (entry == last)
            break;
        entry = entry->next;
    }
    return 1;
}

Dword send_reliable_batch(void *owner, int elapsed) {
    if (reliable_pool_empty(at<ReliablePool *>(owner, 88)))
        return 0;
    auto *pool = at<ReliablePool *>(owner, 88);
    ReliableEntry *head = pool->entries + pool->read % pool->capacity;
    ReliableEntry *entry = head;
    if (entry)
        while (!entry->available) {
            entry->delay -= Dword(elapsed);
            if (int(entry->delay) < 1) {
                if (int(entry->interval) < 64000) {
                    const Dword capacity = Dword(at<std::uint16_t>(owner, 80)) * 3 + 1;
                    auto *packet = static_cast<comm::Packet *>(comm::allocate_object(320));
                    if (packet)
                        comm::construct_packet(packet, nullptr, int(method<Dword>(owner, 19)),
                                               int(capacity));
                    ReliableHeader header{};
                    header.session = at<Dword>(owner, 84);
                    comm::set_endpoint(packet, entry->packet.endpoint);
                    for (;;) {
                        header.sequence = entry->identifier;
                        const Dword count = Dword(comm::available_bytes(&entry->packet));
                        comm::write_packet(packet, comm::packet_data(&entry->packet), int(count));
                        Dword encoded_count = count;
                        encode_comm_header(&encoded_count, "S1");
                        comm::write_packet(packet, &encoded_count, 2);
                        entry->sent_at = comm_milliseconds();
                        const Dword delay = Dword(240) << (entry->retries & 31);
                        ++entry->retries;
                        at<volatile Dword>(entry, 344) = delay;
                        at<volatile Dword>(entry, 340) = delay;
                        header.flags = Byte((header.flags & ~Byte(3)) | ((header.flags + 1) & 3));
                        if ((header.flags & 3) >= 3 || entry == head)
                            break;
                        entry = entry->previous;
                        if (!entry || entry->available)
                            break;
                    }
                    encode_comm_header(&header, "I2");
                    comm::write_packet(packet, &header, 12);
                    method(owner, 16, packet);
                    if (packet)
                        packet->methods[0](packet, 1);
                    return 1;
                }
                clear_reliable_entry(entry);
                at<Dword>(owner, 48) = 0;
                void *connection = at<void *>(owner, 12);
                method(connection, 0, Dword(4), reinterpret_cast<Dword>(connection), Dword(4),
                       connection);
                pool = at<ReliablePool *>(owner, 88);
                reset_reliable_pool(pool, pool->epoch, 0);
                return 0;
            }
            entry = entry->next;
            if (!entry || entry == head)
                break;
        }
    return method<Dword>(owner, 16, static_cast<comm::Packet *>(nullptr));
}
} // namespace nfs::network::session::pending
