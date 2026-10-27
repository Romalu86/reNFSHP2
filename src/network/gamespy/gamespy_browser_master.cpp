#include "gamespy_browser.hpp"
namespace nfs::network::session::pending::browser {
namespace {
int through_separators(char *packet, int size, int separators) {
    char *cursor = packet;
    while (separators) {
        if (cursor == packet + size)
            return -1;
        if (*cursor == '\\')
            --separators;
        ++cursor;
    }
    return int(cursor - packet);
}
} // namespace

int consume_server_row(Browser *owner, char *packet, int size) {
    char text[64];
    if (owner->row_phase == 0) {
        const int size_used = through_separators(packet, size, 3);
        if (size_used == -1)
            return 0;
        if (size_used < 12)
            return -1;
        std::strncpy(text, packet + 12, Dword(size_used) - 12);
        text[size_used - 13] = 0;
        master_column_count = std::atoi(text);
        if (master_column_count >= 21)
            return -1;
        owner->row_phase = 1;
        for (int index = 0; index < master_column_count; ++index)
            master_columns[index] = nullptr;
        return size_used - 1;
    }
    if (owner->row_phase == 1) {
        const int size_used = through_separators(packet, size, 2);
        if (size_used == -1)
            return 0;
        std::strncpy(text, packet + 1, Dword(size_used) - 2);
        text[size_used - 2] = 0;
        char *name = duplicate_string(text);
        enter_hash(owner->strings, &name);
        for (int index = 0; index < master_column_count; ++index)
            if (!master_columns[index]) {
                master_columns[index] = name;
                if (index == master_column_count - 1)
                    owner->row_phase = 2;
                return size_used - 1;
            }
        return -1;
    }
    if (owner->row_phase == 2) {
        const int size_used = through_separators(packet, size, int(Dword(master_column_count) + 1));
        if (size_used == -1)
            return 0;
        Server *server = parse_server_row(master_columns, master_column_count, packet,
                                          owner->query_type, owner->strings);
        http::append_dynamic_array(owner->servers, &server);
        packet[size_used - 1] = '\\';
        owner->callback(owner, 2, owner->context, server, 0);
        return size_used - 1;
    }
    return -1;
}
Dword pump_master_stream(Browser *owner, bool unconditional_cancel) {
    const SOCKET connection = owner->socket;
    fd_set readable;
    readable.fd_count = 1;
    readable.fd_array[0] = connection;
    timeval timeout{0, 0};
    if (select(64, &readable, nullptr, nullptr, &timeout) < 1)
        return 0;
    const int received = recv(connection, reinterpret_cast<char *>(master_buffer + master_bytes),
                              int(2047u - master_bytes), 0);
    if (received == SOCKET_ERROR || !received) {
        closesocket(connection);
        owner->socket = INVALID_SOCKET;
        master_bytes = 0;
        if (unconditional_cancel || owner->phase)
            owner->cancel_requested = 1;
        change_phase(owner, 3);
        return 3;
    }
    if (owner->cipher.depth != -1)
        decrypt(&owner->cipher, master_buffer + master_bytes, received);
    master_bytes += Dword(received);
    Byte *cursor = master_buffer;
    if (owner->cipher.depth == -1 && int(master_buffer[0] ^ 236) < int(master_bytes)) {
        master_buffer[0] ^= 236;
        const int length = int(std::strlen(owner->key));
        for (int index = 0; index < length; ++index)
            master_buffer[1 + index] ^= Byte(owner->key[index]);
        initialize_cipher(master_buffer + 1, master_buffer[0], &owner->cipher);
        cursor = master_buffer + 1 + master_buffer[0];
        decrypt(&owner->cipher, cursor, int(master_bytes) - int(cursor - master_buffer));
    }
    if (owner->cipher.depth != -1) {
        while (int(cursor - master_buffer) <= int(master_bytes - 6u)) {
            if (!std::strncmp(reinterpret_cast<char *>(cursor), "\\final\\", 7) ||
                owner->cancel_requested) {
                closesocket(owner->socket);
                owner->socket = INVALID_SOCKET;
                master_bytes = 0;
                change_phase(owner, owner->query_type == 6 || owner->query_type == 7 ? 0 : 3);
                return 0;
            }
            if (int(master_bytes) < 6)
                break;
            if (owner->query_type == 6 || owner->query_type == 7) {
                const int consumed =
                    consume_server_row(owner, reinterpret_cast<char *>(cursor),
                                       int(master_bytes) - int(cursor - master_buffer));
                if (consumed < 0)
                    owner->cancel_requested = 1;
                else if (!consumed)
                    break;
                else
                    cursor += consumed;
            } else {
                const Dword address = *reinterpret_cast<const Dword *>(cursor);
                const std::uint16_t port = *reinterpret_cast<const std::uint16_t *>(cursor + 4);
                cursor += 6;
                if (find_server(owner, address, ntohs(port), owner->previous_count) == -1)
                    append_server(owner, address, ntohs(port), owner->query_type);
            }
        }
    }
    master_bytes -= Dword(cursor - master_buffer);
    std::memmove(master_buffer, cursor, master_bytes);
    return 0;
}

Dword pump_master(Browser *owner) {
    return pump_master_stream(owner, false);
}

Dword pump(Browser *owner) {
    if (!owner)
        return 5;
    switch (owner->phase) {
    case 1:
        return pump_master_stream(owner, true);
    case 2:
        return pump_lan(owner);
    case 3:
        return pump_queries(owner);
    default:
        return 0;
    }
}
} // namespace nfs::network::session::pending::browser
