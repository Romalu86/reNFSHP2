#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "comm_filter_runtime.hpp"
namespace nfs::network::session::pending {
namespace {
Dword packetizer_marker{}, packetizer_send_guard{}, packetizer_identifier{};
struct ChunkHeader {
    Dword marker, identifier, total_bytes;
    std::uint16_t index, bytes;
    Dword flags;
};
static_assert(sizeof(ChunkHeader) == 20);
} // namespace

void construct_packetizer_signature() {
    packetizer_marker = 1515473744;
}

 
void initialize_packetizer_signature() {
    construct_packetizer_signature();
}


static void reset_outgoing_chunks(void *owner) {
    at<Dword>(owner, 28) = 0;
    std::memset(bytes(owner) + 36, 0, 20);
    at<Byte>(owner, 56) = 0;
    at<Dword>(owner, 60) = 0;
}

static Byte initialize_packetizer(void *owner) {
    at<Dword>(owner, 28) = 0;
    reset_outgoing_chunks(owner);
    for (unsigned offset : {68u, 132u, 136u, 140u, 144u})
        at<Dword>(owner, offset) = 0;
    at<Byte>(owner, 64) = 0;
    return 1;
}

void *construct_packetizer_filter(void *owner) {
    construct_filter(owner, 2);
    for (unsigned offset : {36u, 72u, 92u, 112u}) {
        auto &header = at<ChunkHeader>(owner, offset);
        header.marker = packetizer_marker;
        header.identifier = header.total_bytes = 0;
        header.index = header.bytes = 0;
        header.flags = 0;
    }
    at<const Dword *>(owner, 0) = packetizer_filter_methods;
    initialize_packetizer(owner);
    at<void *>(owner, 148) = allocate_comm_lock();
    return owner;
}

static Dword __fastcall clear_packetizer(void *owner, void *) {
    if (void *buffer = at<void *>(owner, 136)) {
        kernel::memory_free(buffer);
        at<void *>(owner, 136) = nullptr;
    }
    initialize_packetizer(owner);
    return 1;
}

static void destroy_packetizer(void *owner) {
    at<const Dword *>(owner, 0) = packetizer_filter_methods;
    clear_packetizer(owner, nullptr);
    if (void *lock = at<void *>(owner, 148))
        method(lock, 0, Dword(1));
    destroy_filter(owner);
}

static void *__fastcall delete_packetizer(void *owner, void *, Dword flags) {
    destroy_packetizer(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

static void *__fastcall clone_packetizer(void *, void *) {
    void *storage = comm::allocate_object(152);
    return storage ? construct_packetizer_filter(storage) : nullptr;
}
// Complete header-size leaf.
static Dword __fastcall packetizer_header_bytes(void *, void *) {
    return 20;
}

static Byte emit_chunk(void *owner, comm::Packet *source, ChunkHeader *header) {
    if (!source || !comm::packet_data(source))
        return 0;
    const Dword available = Dword(comm::available_bytes(source));
    if (!available)
        return 0;
    const std::uint16_t packet_limit = at<std::uint16_t>(at<void *>(owner, 12), 38);
    const Dword header_bytes = method<Dword>(owner, 19);
    Dword payload = packet_limit - header_bytes;
    if (available < std::uint16_t(payload))
        payload = available;
    alignas(4) Byte endpoint[276];
    std::memcpy(endpoint, source->endpoint, 276);
    endpoint[272] = 1;
    auto *packet = static_cast<comm::Packet *>(comm::allocate_object(320));
    if (packet)
        comm::construct_packet(packet, endpoint, std::uint16_t(header_bytes),
                               std::uint16_t(payload));
    header->bytes =
        std::uint16_t(payload) < available ? std::uint16_t(payload) : std::uint16_t(available);
    comm::write_packet(packet, comm::packet_data(source), header->bytes);
    comm::read_packet(source, nullptr, header->bytes);
    ChunkHeader encoded = *header;
    encode_comm_header(&encoded, "I3S2C4");
    comm::write_packet(packet, &encoded, 20);
    method(owner, 16, packet);
    if (packet)
        packet->methods[0](packet, 1);
    return 1;
}
// Handle every packetizer-filter dispatch case.
static Dword __fastcall send_packetized(void *owner, void *, comm::Packet *packet) {
    if (at<Dword>(owner, 24) == 2)
        return method<Dword>(owner, 16, packet);
    ConnectionOptions options;
    construct_connection_options(&options);
    std::memcpy(&options, bytes(at<void *>(owner, 12)) + 24, 44);
    const Byte queued = at<Byte>(&options, 36);
    if (!queued) {
        if (at<Byte>(owner, 64) && packet) {
            void *connection = at<void *>(owner, 12);
            enter_comm_lock(at<void *>(connection, 96));
            comm::append(at<comm::Mailbox *>(connection, 92), at<Dword>(packet->endpoint, 0),
                         comm::packet_data(packet), Dword(comm::available_bytes(packet)),
                         reinterpret_cast<char *>(packet->endpoint) + 4,
                         at<std::uint16_t>(packet->endpoint, 260));
            leave_comm_lock(at<void *>(connection, 96));
            return 1;
        }
        if (!packetizer_send_guard) {
            packetizer_send_guard = 1;
            drain_connection_queue(at<void *>(owner, 12));
            --packetizer_send_guard;
        }
    }
    if (!packet)
        return method<Dword>(owner, 16, packet);
    reset_outgoing_chunks(owner);
    auto &header = at<ChunkHeader>(owner, 36);
    while (at<Dword>(owner, 28) != 4) {
        switch (at<Dword>(owner, 28)) {
        case 0:
            comm::set_status(packet, 1);
            if (!header.index) {
                header.marker = packetizer_marker;
                header.total_bytes = Dword(comm::available_bytes(packet));
                header.identifier = ++packetizer_identifier;
            }
            at<Dword>(owner, 28) = 1;
            [[fallthrough]];
        case 1:
            at<Dword>(owner, 60) = comm_milliseconds();
            at<Dword>(owner, 28) = emit_chunk(owner, packet, &header) ? 3 : 4;
            break;
        case 3:
            if (comm::available_bytes(packet) > 0) {
                ++header.index;
                at<Dword>(owner, 28) = 0;
            } else {
                comm::set_status(packet, 3);
                at<Dword>(owner, 28) = 4;
            }
            break;
        default:
            _assert("0", "cmn\\module_packetizer.cpp", 341);
            break;
        }
        void *driver = at<void *>(at<void *>(owner, 12), 84);
        if (!driver)
            method(owner, 16, static_cast<comm::Packet *>(nullptr));
        else if (!*comm_options() || queued)
            method(driver, 3, Dword(1));
        kernel::thread_yield(2);
    }
    return 1;
}

static Dword __fastcall receive_packetized(void *owner, void *, comm::Packet *source) {
    if (at<Dword>(owner, 24) == 2 || !source)
        return method<Dword>(owner, 17, source);
    at<Byte>(owner, 64) = 1;
    for (;;) {
        switch (at<Dword>(owner, 68)) {
        case 0: {
            if (!comm::available_bytes(source)) {
                at<Byte>(owner, 64) = 0;
                return 1;
            }
            const int remaining = 20 - at<int>(owner, 132);
            if (remaining > 0) {
                int count = comm::available_bytes(source);
                if (remaining < count)
                    count = remaining;
                comm::read_packet(source, bytes(owner) + 92 + at<Dword>(owner, 132), count);
                at<Dword>(owner, 132) += Dword(count);
            }
            if (at<Dword>(owner, 132) != 20)
                break;
            at<Dword>(owner, 68) = 1;
            [[fallthrough]];
        }
        case 1: {
            auto &incoming = at<ChunkHeader>(owner, 92);
            decode_comm_header(&incoming, "I3S2C4");
            at<Dword>(owner, 132) = 0;
            if (incoming.marker != packetizer_marker) {
                at<Dword>(owner, 68) = 0;
                at<Byte>(owner, 64) = 0;
                return 0;
            }
            auto &previous = at<ChunkHeader>(owner, 72);
            if (at<void *>(owner, 144) && incoming.identifier != previous.identifier &&
                incoming.total_bytes != previous.total_bytes) {
                method(at<void *>(owner, 144), 0, Dword(1));
                at<void *>(owner, 144) = nullptr;
                if (void *buffer = at<void *>(owner, 136)) {
                    kernel::memory_free(buffer);
                    at<void *>(owner, 136) = nullptr;
                    at<Dword>(owner, 140) = 0;
                }
            }
            at<ChunkHeader>(owner, 112) = previous;
            previous = incoming;
            if (at<void *>(owner, 136))
                _assert("m_pInChunkBuffer == 0", "cmn\\module_packetizer.cpp", 488);
            at<void *>(owner, 136) = comm::allocate_named("chunkbuffer", previous.bytes, 1);
            at<Dword>(owner, 140) = 0;
            at<Dword>(owner, 68) = 2;
            [[fallthrough]];
        }
        case 2: {
            if (!comm::available_bytes(source)) {
                at<Byte>(owner, 64) = 0;
                return 1;
            }
            const Dword remaining = at<ChunkHeader>(owner, 72).bytes - at<Dword>(owner, 140);
            if (static_cast<int>(remaining) > 0) {
                Dword count = Dword(comm::available_bytes(source));
                if (static_cast<int>(remaining) < static_cast<int>(count))
                    count = remaining;
                if (static_cast<int>(count) > 0) {
                    if (!at<void *>(owner, 136))
                        _assert("m_pInChunkBuffer", "cmn\\module_packetizer.cpp", 520);
                    std::memmove(bytes(at<void *>(owner, 136)) + at<Dword>(owner, 140),
                                 comm::packet_data(source), count);
                    at<Dword>(owner, 140) += count;
                    comm::read_packet(source, nullptr, int(count));
                }
            }
            if (at<ChunkHeader>(owner, 72).bytes > at<Dword>(owner, 140))
                break;
            at<Dword>(owner, 68) = 3;
            [[fallthrough]];
        }
        case 3: {
            auto &current = at<ChunkHeader>(owner, 72);
            auto &previous = at<ChunkHeader>(owner, 112);
            if (current.identifier == previous.identifier && current.index == previous.index)
                at<Dword>(owner, 68) = 0;
            else {
                auto *packet = at<comm::Packet *>(owner, 144);
                if (!packet) {
                    packet = static_cast<comm::Packet *>(comm::allocate_object(320));
                    if (packet)
                        comm::construct_packet(packet, nullptr, 0, int(current.total_bytes));
                    at<comm::Packet *>(owner, 144) = packet;
                    comm::set_mode(packet, 1);
                }
                comm::write_packet(packet, at<void *>(owner, 136), int(at<Dword>(owner, 140)));
                at<Dword>(owner, 68) =
                    Dword(comm::available_bytes(packet)) < current.total_bytes ? 0 : 4;
            }
            kernel::memory_free(at<void *>(owner, 136));
            at<void *>(owner, 136) = nullptr;
            at<Dword>(owner, 140) = 0;
            if (at<Dword>(owner, 68) != 4)
                break;
            [[fallthrough]];
        }
        case 4: {
            at<Dword>(owner, 68) = 0;
            auto *pending = at<comm::Packet *>(owner, 144);
            if (!pending)
                _assert("m_pPendingInPacket", "cmn\\module_packetizer.cpp", 605);
            auto *packet = static_cast<comm::Packet *>(comm::allocate_object(320));
            if (packet)
                comm::construct_packet_copy(packet, pending);
            if (pending)
                pending->methods[0](pending, 1);
            at<void *>(owner, 144) = nullptr;
            method(owner, 17, packet);
            if (packet)
                packet->methods[0](packet, 1);
            break;
        }
        default:
            _assert("0", "cmn\\module_packetizer.cpp", 631);
            break;
        }
    }
}
const Dword packetizer_filter_methods[]{reinterpret_cast<Dword>(delete_packetizer),
                                       reinterpret_cast<Dword>(filter_kind),
                                       reinterpret_cast<Dword>(filter_mode),
                                       reinterpret_cast<Dword>(set_filter_mode),
                                       reinterpret_cast<Dword>(enable_filter),
                                       reinterpret_cast<Dword>(disable_filter),
                                       reinterpret_cast<Dword>(clear_packetizer),
                                       reinterpret_cast<Dword>(clone_packetizer),
                                       reinterpret_cast<Dword>(filter_connection),
                                       reinterpret_cast<Dword>(set_filter_connection),
                                       reinterpret_cast<Dword>(filter_previous),
                                       reinterpret_cast<Dword>(set_filter_previous),
                                       reinterpret_cast<Dword>(filter_next),
                                       reinterpret_cast<Dword>(set_filter_next),
                                       reinterpret_cast<Dword>(send_packetized),
                                       reinterpret_cast<Dword>(receive_packetized),
                                       reinterpret_cast<Dword>(filter_send),
                                       reinterpret_cast<Dword>(filter_receive),
                                       reinterpret_cast<Dword>(packetizer_header_bytes),
                                       reinterpret_cast<Dword>(filter_total_header_bytes)};
} // namespace nfs::network::session::pending

NFS_KERNEL_INITIALIZER(startup_initialize_packetizer_signature, ".CRT$XCU_NFS_00690738",
                       nfs::network::session::pending::initialize_packetizer_signature);
