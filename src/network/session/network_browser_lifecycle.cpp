#include "network_lifecycle_runtime.hpp"
#include "nfs/frontend/network/frontend_network.hpp"
namespace nfs::network::session {
Byte internet_query_active{};
Dword &browser_started = *reinterpret_cast<Dword *>(frontend::state + 860);
namespace {
Dword query_clock{}, query_clock_guard{};
} // namespace

void update_internet_queries() {
    if (!(query_clock_guard & 1)) {
        query_clock_guard |= 1;
        query_clock = GetTickCount();
    }
    const Dword now = GetTickCount(), delta = now - query_clock;
    const Dword sign = Dword(static_cast<std::int32_t>(delta) >> 31);
    if (static_cast<std::int32_t>((delta ^ sign) - sign) > 100) {
        query_clock = now;
        pending::browser_think();
    }
}
} // namespace nfs::network::session
namespace nfs::services::pending {

void tick_server_browser() {
    if (GetTickCount() - network::session::browser_started < 10000)
        network::session::update_internet_queries();
}

void tick_internet_services() {
    if (network::session::internet_query_active)
        network::session::update_internet_queries();
}
} // namespace nfs::services::pending
