#include "frontend_network_session.hpp"
#include "../../network/session/network_lifecycle_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using game::runtime::at;

char *parse_network_argument(char *text, int *consumed, const char *stop, const char *leading,
                             const char *trailing) {
    if (!text)
        return nullptr;
    const int length = int(stop ? std::strcspn(text, stop) : std::strlen(text));
    char *end = text + length;
    if (leading)
        text += std::strspn(text, leading);
    if (trailing)
        while (text < end && std::strchr(trailing, end[-1]))
            --end;
    *consumed = length + 1;
    *end = 0;
    return *text ? text : nullptr;
}

void execute_server_setting(NetworkSessionWidget *widget, char *begin, char *limit) {
    auto report = [&](Dword reference) {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, reference);
        session_notice(widget, &text.value);
    };
    int consumed;
    char *argument = parse_network_argument(begin, &consumed, " ", " ", nullptr);
    if (!argument) {
        report(1642);
        return;
    }
    char key[6];
    std::strncpy(key, argument, 5);
    key[5] = 0;
    char *value_text = begin + consumed;
    if (reinterpret_cast<Dword>(limit) < reinterpret_cast<Dword>(value_text)) {
        report(1642);
        return;
    }
    if (!*value_text) {
        report(1643);
        return;
    }
    value_text = parse_network_argument(value_text, &consumed, nullptr, " ", nullptr);
    if (!value_text || reinterpret_cast<Dword>(limit) < reinterpret_cast<Dword>(value_text)) {
        report(1643);
        return;
    }
    int value = std::atoi(value_text);
    auto *server = network::server();
    unsigned offset;
    if (!_strnicmp(key, "finishtimeout", 4))
        offset = 132;
    else if (!_strnicmp(key, "laptimeout", 4)) {
        offset = 128;
        if (value && value < 300000)
            value = 300000;
    } else if (!_strnicmp(key, "maxsetuptime", 4)) {
        offset = 160;
        if (value && value < 15000)
            value = 15000;
    } else if (!_strnicmp(key, "heartbeat", 4)) {
        offset = 104;
        if (value && value < 10000)
            value = 10000;
    } else if (!_strnicmp(key, "resultsdisplaytime", 4))
        offset = 112;
    else if (!_strnicmp(key, "countdown", 4)) {
        offset = 164;
        if (value && value < 5000)
            value = 5000;
    } else if (!_strnicmp(key, "docountdown", 4))
        offset = 168;
    else if (!_strnicmp(key, "voting", 4))
        offset = 140;
    else if (!_strnicmp(key, "numraces", 4)) {
        offset = 76;
        if (value < 1)
            value = 1;
    } else if (!_strnicmp(key, "laps", 4)) {
        offset = 72;
        if (value < 0)
            value = -1;
        else if (value > 9)
            value = 9;
    } else if (!_strnicmp(key, "class", 4)) {
        offset = 60;
        value = value >= 1 && value <= 5 ? value + 37 : 6;
    } else if (!_strnicmp(key, "direction", 4)) {
        offset = 68;
        if (value < 1 || value > 4)
            value = 0;
    } else
        return;
    at<int>(server, offset) = value;
    report(1644);
}
} // namespace nfs::application::frontend
