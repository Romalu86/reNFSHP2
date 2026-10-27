#include "nfs/network/packets/network_authentication.hpp"
#include "nfs/network/packets/network_digest.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cctype>
namespace nfs::network::authentication {
using namespace packets;

void set_status(Dword *record, Dword status) {
    record[1] = status;
    field<Dword>(record, 320) = GetTickCount();
}

void append_record(Dword *record) {
    for (auto &slot : packets::pending::authentication_records)
        if (!slot) {
            slot = record;
            return;
        }
}

void release_record(int index) {
    gui::application_release(packets::pending::authentication_records[index]);
    packets::pending::authentication_records[index] = nullptr;
}

void unmark_records() {
    for (auto *record : packets::pending::authentication_records)
        if (record)
            field<std::uint8_t>(record, 324) = 0;
}

void send_response(Dword *owner, const char *response) {
    alignas(4) Byte packet[104];
    packet[0] = 30;
    std::strcpy(reinterpret_cast<char *>(packet + 4), response);
    at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 376);
    send(bytes(owner) + 72, packet, sizeof(packet));
}

Dword player_address(Dword *owner, int identifier) {
    ui::PlayerRecord player{};
    Dword route[76];
    if (copy_player(&identifier, &player) &&
        queue::copy_matching(route_queue(owner), address_of(bytes(&player) + 2), address_of(route),
                             nullptr) &&
        route[9]) {
        void *endpoint = network::address(reinterpret_cast<void *>(route[9]));
        if (endpoint)
            return at<Dword>(endpoint, 4);
    }
    return 0;
}

void reject_player(Dword *owner, Dword identifier, Dword reason) {
    ui::PlayerRecord player{};
    Dword route[76];
    if (copy_player(&identifier, &player) &&
        queue::copy_matching(route_queue(owner), address_of(bytes(&player) + 2), address_of(route),
                             nullptr)) {
        while (remove_server_client(owner, nullptr, route[0])) {
        }
        alignas(4) Byte packet[8];
        packet[0] = 16;
        at<Dword>(packet, 4) = reason;
        send(route, packet, 8);
    }
}
// Exact salt/response construction; no bypass of key admission.
void make_response(const char *key, const char *challenge, char *response) {
    const Dword key_length = Dword(std::strlen(key)), challenge_length = Dword(std::strlen(challenge));
    if (challenge_length + 8 + key_length * 2 > 511) {
        std::strcpy(response, "CD Key or challenge too long");
        return;
    }
    __time32_t now = _time32(nullptr);
    std::srand(Dword(now) ^ 858993459u);
    const int high = std::rand();
    const Dword salt = (Dword(high) << 16) | Dword(std::rand());
    char salt_text[12], input[512];
    std::sprintf(salt_text, "%.8x", salt);
    std::sprintf(input, "%s%d%s", key, salt % 65535, challenge);
    digest::hexadecimal(key, key_length, response);
    std::strcpy(response + 32, salt_text);
    digest::hexadecimal(input, Dword(std::strlen(input)), response + 40);
}
// The caller supplies four cdecl arguments.
void __cdecl complete(Dword identifier, int status, const char *message, void *) {
    if (auto *record = packets::pending::authentication_record(identifier)) {
        if (status == 1)
            set_status(record, 3);
        else {
            set_status(record, 4);
            set_error(record, message);
            reject_player(server(), record[0], 18);
        }
    }
}
} // namespace nfs::network::authentication
namespace nfs::network::packets::pending {

void answer_authentication_challenge(void *challenge) {
    char response[73]{};
    if (authentication::valid_key(authentication::client_key))
        authentication::make_response(authentication::client_key, static_cast<char *>(challenge),
                                      response);
    authentication::send_response(client(), response);
}
// The second caller argument is unused by the body.
void generate_client_key(char *output, int) {
    char result[33]{};
    digest::hexadecimal(authentication::client_key, Dword(std::strlen(authentication::client_key)),
                        result);
    std::strcpy(output, result);
}

void receive_authentication(int identifier, void *input) {
    auto *record = authentication_record(identifier);
    if (!record)
        return;
    auto *response = static_cast<char *>(input);
    if (std::strlen(response) < 72) {
        authentication::set_status(record, 4);
        authentication::set_error(record, "Bad CD Key");
        authentication::reject_player(server(), record[0], 18);
        return;
    }
    char key[33]{};
    std::memcpy(key, response, 32);
    Dword reason = 0;
    char error[300]{};
    if (!validate_key(key, &reason, error)) {
        authentication::set_status(record, 4);
        authentication::set_error(record, error);
        authentication::reject_player(server(), record[0], reason);
        return;
    }
    authentication::set_status(record, 2);
    authentication::set_key(record, key);
    if (session_kind == 1) {
        authentication::pending::submit_authentication_challenge(
            identifier, authentication::player_address(server(), identifier),
            reinterpret_cast<const char *>(record + 2), response, authentication::complete,
            nullptr);
        return;
    }
    if (auto *current = authentication_record(identifier))
        authentication::set_status(current, 3);
}
} // namespace nfs::network::packets::pending
