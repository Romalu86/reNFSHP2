#include "../session/network_lifecycle_runtime.hpp"
#include <cstdlib>
namespace nfs::network::session {
namespace {
Dword authentication_clock{}, authentication_clock_guard{};
Dword absolute_delta(Dword value) {
    const Dword sign = Dword(static_cast<std::int32_t>(value) >> 31);
    return (value ^ sign) - sign;
}
} // namespace

void initialize_authentication() {
    pending::initialize_key_service(0, 659);
}

static void set_challenge(Dword *record, const char *challenge) {
    std::strcpy(reinterpret_cast<char *>(record) + 8, challenge);
}
namespace pending {
void send_challenge(Dword *, Dword, const char *);
void remove_key_client(Dword);

void send_authentication_challenge(Dword *record) {
    char challenge[9]{};
    for (unsigned index = 0; index < 8;) {
        const Byte value = Byte(std::rand());
        if ((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
            (value >= '0' && value <= '9'))
            challenge[index++] = char(value);
    }
    set_challenge(record, challenge);
    send_challenge(server(), record[0], challenge);
}

void release_unmarked_authentication() {
    for (auto *&record : packets::pending::authentication_records)
        if (record && !at<Byte>(record, 324)) {
            remove_key_client(record[0]);
            gui::application_release(record);
            record = nullptr;
        }
}
} // namespace pending

void update_authentication() {
    int pending_count = 0;
    for (auto *record : packets::pending::authentication_records)
        if (record && record[1] == 2)
            ++pending_count;
    if (pending_count > 0) {
        if (!(authentication_clock_guard & 1)) {
            authentication_clock_guard |= 1;
            authentication_clock = GetTickCount();
        }
        const Dword now = GetTickCount();
        if (static_cast<std::int32_t>(absolute_delta(now - authentication_clock)) > 100) {
            authentication_clock = now;
            pending::update_key_service();
        }
    }
    authentication::unmark_records();
    pending::mark_authentication_players(player_queue());
    pending::release_unmarked_authentication();
    for (auto *record : packets::pending::authentication_records) {
        if (!record)
            continue;
        if (record[1] == 0) {
            pending::send_authentication_challenge(record);
            authentication::set_status(record, 1);
        } else if (record[1] == 1) {
            const Dword now = GetTickCount();
            if (static_cast<std::int32_t>(absolute_delta(now - at<Dword>(record, 320))) > 30000) {
                authentication::set_status(record, 4);
                authentication::set_error(record, "Timed Out");
                authentication::reject_player(server(), record[0], 18);
            }
        }
    }
}
} // namespace nfs::network::session
