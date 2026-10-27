#include "gamespy_query_runtime.hpp"
namespace nfs::network::session::pending {
namespace {
char query_values[2][256]{};
Dword query_value_slot{};
using QueryCallback = void(__cdecl *)(char *, int, Dword);
void append_callback(void *owner, unsigned offset, sockaddr *endpoint, char *packet) {
    char reply[1400]{};
    at<QueryCallback>(owner, offset)(reply, 1400, at<Dword>(owner, 256));
    append_query(owner, endpoint, packet, reply);
}
} // namespace

void flush_query(void *owner, sockaddr *endpoint, char *packet) {
    if (*packet) {
        ++at<Dword>(owner, 224);
        char identifier[80];
        std::sprintf(identifier, "\\queryid\\%d.%d", at<Dword>(owner, 220), at<Dword>(owner, 224));
        std::strcat(packet, identifier);
        sendto(at<SOCKET>(owner, 0), packet, int(std::strlen(packet)), 0, endpoint, 16);
        *packet = 0;
    }
}

void append_query(void *owner, sockaddr *endpoint, char *packet, char *text) {
    while (std::strlen(packet) + std::strlen(text) > 1349) {
        if (std::strlen(text) < 1351) {
            flush_query(owner, endpoint, packet);
            std::strcpy(packet, text);
            return;
        }
        unsigned delimiters = 0;
        char *split = text;
        for (unsigned i = 0; i < 1350; ++i)
            if (text[i] == '\\' && !(delimiters++ & 1))
                split = text + i;
        if (split == text)
            return;
        *split = 0;
        append_query(owner, endpoint, packet, text);
        *split = '\\';
        text = split;
    }
    std::strcat(packet, text);
}

void query_information(void *owner, sockaddr *endpoint, char *packet) {
    append_callback(owner, 204, endpoint, packet);
}

void query_rules(void *owner, sockaddr *endpoint, char *packet) {
    append_callback(owner, 208, endpoint, packet);
}

void query_players(void *owner, sockaddr *endpoint, char *packet) {
    append_callback(owner, 212, endpoint, packet);
}

void query_echo(void *owner, sockaddr *endpoint, char *packet, const char *echo) {
    if (std::strlen(echo) < 1351) {
        char reply[1400];
        std::sprintf(reply, "\\echo\\%s", echo);
        append_query(owner, endpoint, packet, reply);
    }
}

void encode_query_validation(const Byte *input, int length, char *output) {
    static constexpr char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int consumed = 0;
    while (consumed < length) {
        Byte group[3];
        for (unsigned i = 0; i < 3; ++i, ++consumed)
            group[i] = consumed < length ? *input++ : 0;
        *output++ = alphabet[group[0] >> 2];
        *output++ = alphabet[((group[0] & 3) << 4) + (group[1] >> 4)];
        *output++ = alphabet[((group[1] & 15) << 2) + (group[2] >> 6)];
        *output++ = alphabet[group[2] & 63];
    }
    *output = 0;
}

void encrypt_query_validation(const Byte *key, int key_length, Byte *data, int length) {
    Byte state[256];
    for (unsigned i = 0; i < 256; ++i)
        state[i] = Byte(i);
    Dword key_index = 0, swap_index = 0;
    for (unsigned i = 0; i < 256; ++i) {
        const Dword index = key_index & 255;
        key_index = Dword(static_cast<std::int64_t>(index + 1) % key_length);
        swap_index = (state[i] + swap_index + key[index]) & 255;
        const Byte value = state[i];
        state[i] = state[swap_index];
        state[swap_index] = value;
    }
    Dword first = 0, second = 0;
    std::int16_t offset = 0;
    while (int(offset) < length) {
        const Byte value = data[offset];
        first = (value + 1 + first) & 255;
        second = (state[first] + second) & 255;
        const Byte held = state[first];
        state[first] = state[second];
        state[second] = held;
        data[offset] = state[(state[first] + state[second]) & 255] ^ value;
        offset = std::int16_t(std::uint16_t(offset) + 1);
    }
}

void finish_query(void *owner, sockaddr *endpoint, char *packet, const char *secure) {
    char reply[256];
    if (*secure) {
        const int length = int(std::strlen(secure));
        if (length > 128)
            return;
        char encrypted[128];
        char encoded[200];
        std::strcpy(encrypted, secure);
        encrypt_query_validation(reinterpret_cast<Byte *>(query_text(owner, 72)),
                                 int(std::strlen(query_text(owner, 72))),
                                 reinterpret_cast<Byte *>(encrypted), length);
        encode_query_validation(reinterpret_cast<Byte *>(encrypted), length, encoded);
        std::sprintf(reply, "\\validate\\%s", encoded);
        append_query(owner, endpoint, packet, reply);
    }
    std::sprintf(reply, "\\final\\");
    append_query(owner, endpoint, packet, reply);
    flush_query(owner, endpoint, packet);
}

void dispatch_query(void *owner, char *datagram, Dword length, sockaddr *endpoint) {
    char packet[1400]{};
    char secure[256]{};
    if (!owner)
        owner = query_context;
    if (*datagram == ';') {
        if (auto callback = at<void(__cdecl *)(char *, Dword, sockaddr *)>(owner, 252))
            callback(datagram, length, endpoint);
        return;
    }
    ++at<Dword>(owner, 220);
    at<Dword>(owner, 224) = 0;
    if (at<Byte>(owner, 232))
        at<Byte>(owner, 232) = 0;
    static constexpr const char *keys[]{"basic",  "info",    "rules", "players",
                                        "status", "packets", "echo",  "secure"};
    for (unsigned kind = 0; kind < 8; ++kind) {
        query_value_slot ^= 1;
        char pattern[256] = "\\";
        std::strcat(pattern, keys[kind]);
        std::strcat(pattern, "\\");
        char *found = std::strstr(datagram, pattern);
        if (!found)
            continue;
        found += std::strlen(pattern);
        char *value = query_values[query_value_slot];
        char *end = value;
        while (*found && *found != '\\')
            *end++ = *found++;
        *end = 0;
        switch (kind) {
        case 0:
            append_callback(owner, 200, endpoint, packet);
            break;
        case 1:
            query_information(owner, endpoint, packet);
            break;
        case 2:
            query_rules(owner, endpoint, packet);
            break;
        case 3:
            query_players(owner, endpoint, packet);
            break;
        case 4:
            append_callback(owner, 200, endpoint, packet);
            query_information(owner, endpoint, packet);
            query_rules(owner, endpoint, packet);
            query_players(owner, endpoint, packet);
            break;
        case 5:
            append_callback(owner, 200, endpoint, packet);
            flush_query(owner, endpoint, packet);
            query_information(owner, endpoint, packet);
            flush_query(owner, endpoint, packet);
            query_rules(owner, endpoint, packet);
            flush_query(owner, endpoint, packet);
            query_players(owner, endpoint, packet);
            break;
        case 6:
            query_echo(owner, endpoint, packet, value);
            break;
        case 7:
            std::strcpy(secure, value);
            break;
        }
    }
    finish_query(owner, endpoint, packet, secure);
}
} // namespace nfs::network::session::pending
