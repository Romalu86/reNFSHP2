#include "gamespy_updates.hpp"
#include "gamespy_http_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include <ctime>
#include <algorithm>
namespace nfs::network::session::pending::http {
Dword submit_memory_request(const char *, const char *, char *, int, void *, Dword, Dword, Dword, Dword,
                           Dword);

Dword submit_download(const char *url, Dword synchronous, Dword complete, Dword context) {
    return submit_memory_request(url, nullptr, nullptr, 0, nullptr, 0, synchronous, 0, complete,
                                 context);
}
} // namespace nfs::network::session::pending::http
namespace nfs::network::session::pending::updates {
namespace {
char identifier[260]{};
char field[256]{};
char request_url[256]{};
Dword identifier_seed = 1;
struct Request {
    Callback callback;
    Dword context;
};
static_assert(sizeof(Request) == 8);
const char *read_field(const char *response, const char *key) {
    const char *value = std::strstr(response, key);
    if (!value)
        return nullptr;
    value += std::strlen(key);
    const auto length = (std::min)(std::strcspn(value, "\\"), std::size_t(255));
    std::memcpy(field, value, length);
    field[length] = 0;
    return field;
}
} // namespace

static const char *client_identifier() {
    static constexpr const char *path = "Software\\GameSpy\\GameSpy 3D\\Registration";
    HKEY key;
    DWORD type = REG_SZ, size = sizeof(identifier), disposition;
    const LSTATUS opened = RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &key);
    const LSTATUS queried =
        RegQueryValueExA(key, "Crypt", nullptr, &type, reinterpret_cast<BYTE *>(identifier), &size);
    if (queried || std::strlen(identifier) != 19) {
        char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        LARGE_INTEGER counter;
        const Dword counter_bits =
            QueryPerformanceCounter(&counter) ? counter.HighPart ^ counter.LowPart : 0;
        const Dword wall = Dword(std::time(nullptr));
        const Dword clock = game::world::pending::legacy_clock_entry();
        Dword seed = wall ^ clock ^ GetTickCount() ^ counter_bits;
        identifier_seed = seed ? seed & 2147483647u : 1;
        for (int index = 0; index < 19; ++index) {
            if (index == 4 || index == 9 || index == 14)
                identifier[index] = '-';
            else {
                const Dword high = (identifier_seed >> 16) * 16807;
                seed = (identifier_seed & 65535) * 16807 + (high & 32767) * 65536;
                if (seed > 2147483647)
                    seed = (seed & 2147483647) + 1;
                seed += high >> 15;
                if (seed > 2147483647)
                    seed = (seed & 2147483647) + 1;
                identifier_seed = seed;
                identifier[index] = alphabet[std::int32_t(seed) % 62];
            }
        }
        identifier[19] = 0;
        if (opened)
            RegCreateKeyExA(HKEY_CURRENT_USER, path, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key,
                            &disposition);
        RegSetValueExA(key, "Crypt", 0, REG_SZ, reinterpret_cast<const BYTE *>(identifier),
                       DWORD(std::strlen(identifier) + 1));
    }
    RegCloseKey(key);
    std::memmove(identifier + 4, identifier + 5, 4);
    std::memmove(identifier + 8, identifier + 10, 4);
    std::memmove(identifier + 12, identifier + 15, 4);
    identifier[16] = 0;
    return identifier;
}

static Dword __cdecl version_received(Dword, int result, const char *response, Dword,
                                     Request *request) {
    bool available = false;
    Dword lockout = 0, file_id = 0;
    char name[256], url[101];
    if (!result) {
        const char *value = read_field(response, "\\newver\\");
        if (value && std::atoi(value)) {
            available = true;
            value = read_field(response, "\\lockout\\");
            lockout = value && std::atoi(value) != 0;
            value = read_field(response, "\\fpfileid\\");
            file_id = value ? Dword(std::atoi(value)) : 0;
            value = read_field(response, "\\newvername\\");
            if (!value)
                name[0] = 0;
            else {
                std::strncpy(name, value, 256);
                name[255] = 0;
            }
            value = read_field(response, "\\dlurl\\");
            if (!value)
                url[0] = 0;
            else {
                std::strncpy(url, value, 101);
                url[100] = 0;
            }
        }
    }
    if (request->callback) {
        if (available)
            request->callback(1, lockout, name, file_id, url, request->context);
        else
            request->callback(0, 0, "", 0, "", request->context);
    }
    std::free(request);
    return 1;
}

Dword request_version(Dword product, const char *version, Dword distribution, Callback callback,
                     Dword synchronous, Dword context) {
    if (!version || !callback)
        return 0;
    auto *request = static_cast<Request *>(std::malloc(8));
    if (!request)
        return 0;
    request->callback = nullptr;
    request->context = 0;
    request->callback = callback;
    request->context = context;
    std::sprintf(
        request_url,
        "http://motd.gamespy.com/motd/vercheck.asp?productid=%d&versionuniqueid=%s&distid=%d",
        product, version, distribution);
    const Dword handle =
        http::submit_download(request_url, synchronous, reinterpret_cast<Dword>(version_received),
                              reinterpret_cast<Dword>(request));
    if (handle == Dword(-1) && !synchronous)
        return 0;
    return 1;
}

Dword request_motd(Dword user, Dword product, const char *version, Dword distribution,
                  Dword synchronous) {
    if (!version)
        return 0;
    const char *unique_id = client_identifier();
    std::sprintf(request_url,
                 "http://motd.gamespy.com/motd/"
                 "motd.asp?userid=%d&productid=%d&versionuniqueid=%s&distid=%d&uniqueid=%s",
                 user, product, version, distribution, unique_id);
    return http::submit_download(request_url, synchronous, 0, 0) != Dword(-1);
}
} // namespace nfs::network::session::pending::updates
