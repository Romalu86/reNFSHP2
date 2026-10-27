#include "comm_connection_runtime.hpp"
namespace nfs::network::session::pending {
// Variadic diagnostic is a complete RET leaf.
static void trace_communication(const char *, ...) {
}

void *bottom_connection_filter(void *owner) {
    void *filter = at<void *>(owner, 88);
    while (filter && method<void *>(filter, 12))
        filter = method<void *>(filter, 12);
    return filter;
}

Dword link_filter_modules(void *upper, void *lower) {
    if (upper)
        method(upper, 13, lower);
    if (lower)
        method(lower, 11, upper);
    return 1;
}

Dword link_connection_filters(void *owner, void *filter) {
    if (!at<void *>(owner, 88)) {
        at<void *>(owner, 88) = filter;
        method(filter, 9, owner);
        return 1;
    }
    void *bottom = bottom_connection_filter(owner);
    if (!bottom)
        _assert("pBottomModule", "cmn\\connection2.cpp", 48);
    link_filter_modules(bottom, filter);
    method(filter, 9, owner);
    return 1;
}

Dword unlink_connection_filter(void *owner, void *filter) {
    if (!filter) {
        comm::set_error(3);
        return 0;
    }
    if (method<void *>(filter, 8) != owner)
        return 0;
    enter_comm_lock(at<void *>(owner, 80));
    void *next = method<void *>(filter, 12);
    void *previous = method<void *>(filter, 10);
    link_filter_modules(previous, next);
    method(filter, 9, static_cast<void *>(nullptr));
    leave_comm_lock(at<void *>(owner, 80));
    return 1;
}

Dword clear_connection_filters(void *owner) {
    void *filter = at<void *>(owner, 88);
    while (filter) {
        void *next = method<void *>(filter, 12);
        unlink_connection_filter(owner, filter);
        method(filter, 0, Dword(1));
        filter = next;
    }
    at<void *>(owner, 88) = nullptr;
    return 1;
}

Dword deliver_connection_packet(void *owner, comm::Packet *packet) {
    if (packet) {
        if (void *callback = at<void *>(owner, 76)) {
            const int length = comm::available_bytes(packet);
            void *data = comm::packet_data(packet);
            method(callback, 0, Dword(1), reinterpret_cast<Dword>(data), Dword(length), owner);
            return 1;
        }
        _assert("0", "cmn\\connection2.cpp", 541);
    }
    return 0;
}

Dword __fastcall connection_event(void *owner, void *, Dword event, Dword first, Dword second, void *) {
    if (event >= 1004)
        return 0;
    if (event == 1003) {
        at<void *>(owner, 84) = nullptr;
        return 0;
    }
    void *callback = at<void *>(owner, 76);
    switch (event) {
    case 1:
        if (first && second) {
            auto *packet = static_cast<comm::Packet *>(comm::allocate_object(320));
            if (packet)
                comm::construct_packet(packet, nullptr, 0, int(second));
            comm::write_packet(packet, reinterpret_cast<void *>(first), int(second));
            if (void *filter = bottom_connection_filter(owner))
                method(filter, 15, packet);
            else
                deliver_connection_packet(owner, packet);
            if (packet)
                packet->methods[0](packet, 1);
        }
        break;
    case 2:
        _assert("0", "cmn\\connection.cpp", 820);
        break;
    case 3:
    case 4:
        swap_error(static_cast<Dword *>(owner), event == 3 ? 4 : 0);
        if (callback)
            method(callback, 0, event, Dword(0), Dword(0), owner);
        break;
    case 5:
        if (at<Dword>(owner, 68) == 4) {
            swap_error(static_cast<Dword *>(owner), 1);
            if (callback)
                method(callback, 0, Dword(5), Dword(0), Dword(0), owner);
        }
        break;
    case 6: {
        auto *packet = reinterpret_cast<comm::Packet *>(first);
        if (at<Byte>(packet->endpoint, 272)) {
            trace_communication("Internal packet send complete!\n");
            break;
        }
        if (callback)
            method(callback, 0, Dword(6), at<Dword>(packet->endpoint, 0), Dword(0), owner);
        break;
    }
    case 7:
    case 9:
    case 10:
        if (callback)
            method(callback, 0, event, first, second, owner);
        break;
    }
    return 0;
}

Dword send_connection_data(void *owner, const void *data, Dword count, const char *address,
                          std::uint16_t port) {
    if (static_cast<int>(count) < 1)
        return 0;
    if (!++comm::sequence)
        comm::sequence = 1;
    Dword identifier = comm::sequence;
    if (!at<Byte>(owner, 60)) {
        comm::send_immediate(static_cast<Dword *>(owner), identifier, data, count, address, port);
        return identifier;
    }
    enter_comm_lock(at<void *>(owner, 96));
    auto *mailbox = at<comm::Mailbox *>(owner, 92);
    if (count >= comm::available(mailbox, nullptr) ||
        !comm::append(mailbox, identifier, data, count, address, port)) {
        comm::set_error(23);
        identifier = 0;
        if (void *callback = at<void *>(owner, 76))
            method(callback, 0, Dword(9), Dword(23), Dword(4), owner);
    }
    leave_comm_lock(at<void *>(owner, 96));
    return identifier;
}

Dword __fastcall send_connection(void *owner, void *, const void *data, Dword count) {
    return send_connection_data(owner, data, count, nullptr, 0);
}

int drain_connection_queue(void *owner) {
    if (void *driver = at<void *>(owner, 84))
        if (!method<Byte>(driver, 7, owner))
            return 0;
    enter_comm_lock(at<void *>(owner, 96));
    auto *mailbox = at<comm::Mailbox *>(owner, 92);
    leave_comm_lock(at<void *>(owner, 96));
    int sent = 0;
    for (auto *entry = mailbox->head; entry;) {
        const Dword count = entry->bytes, identifier = entry->identifier;
        const Dword port = comm::entry_port(entry);
        const char *address = comm::entry_address(entry);
        void *data = comm::entry_data(entry);
        if (!comm::send_immediate(static_cast<Dword *>(owner), identifier, data, count, address,
                                  std::uint16_t(port)))
            break;
        comm::pop(mailbox);
        ++sent;
        entry = comm::entry_next(entry);
    }
    return sent;
}

Dword __fastcall update_connection(void *owner, void *) {
    comm::send_via_filter(static_cast<Dword *>(owner), nullptr);
    drain_connection_queue(owner);
    return 1;
}
Dword __fastcall close_connection_method(void *owner, void *) {
    return close_connection(owner);
}
} // namespace nfs::network::session::pending
