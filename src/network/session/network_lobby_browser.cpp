#include "../../frontend/flow/frontend_multiplayer.hpp"
#include "server_runtime.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/network/packets/network_authentication.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::application::pending {
void *construct_server_settings(void *);
}
namespace nfs::network::lobby {
using game::runtime::at;
using game::runtime::bytes;
namespace ui = application::frontend;
sdk::Browser *browser{};
Dword refresh_started{}, query_phase{}, direct_port{};
char direct_address[500]{};
Byte queued_lan{}, queued_master{}, queued_direct{}, test_master{};
Byte enforce_version = 1, check_version = 1, request_motd = 1;
namespace {
Dword lan_clock{}, lan_clock_guard{};
int mode() {
    return packets::pending::session_kind;
}
bool automatic_bandwidth() {
    return !application::pending::server_password_enabled;
}
void clear_visible_servers() {
    auto *screen = ui::multiplayer_screen;
    if (screen && ui::current_browser(screen))
        ui::clear_browser_servers(ui::current_browser(screen));
}
void arm_mode() {
    if (mode() == 1) {
        if (request_motd)
            send_motd();
        if (check_version)
            check_for_update(0, 1);
        if (automatic_bandwidth() && !bandwidth::probe_phase)
            bandwidth::initialize();
        queued_lan = queued_master = 1;
    } else if (!mode())
        queued_lan = 1;
    else if (mode() == 2 && automatic_bandwidth() && !bandwidth::probe_phase)
        bandwidth::initialize();
}
} // namespace

void initialize() {
    const char *game = test_master ? "ut" : "nfs6";
    const char *query_game = test_master ? "gmtest" : "nfs6";
    const char *key = test_master ? "HA6zkS" : session::service_key;
    browser = sdk::create(game, query_game, key, services::dependency::maximum_concurrent_updates,
                          browser_event, 1, 0);
    packets::pending::session_kind = 0;
    queued_lan = queued_master = queued_direct = 0;
    if (query_phase) {
        sdk::cancel(browser);
        query_phase = 3;
    }
    sdk::clear(browser);
    clear_visible_servers();
    if (mode() == 1) {
        if (request_motd)
            send_motd();
        if (check_version)
            check_for_update(0, 1);
        if (automatic_bandwidth() && !bandwidth::probe_phase)
            bandwidth::initialize();
        refresh_started = query_phase = 0;
        queued_lan = queued_master = 1;
        return;
    }
    if (!mode()) {
        refresh_started = query_phase = 0;
        queued_lan = 1;
        return;
    }
    if (mode() == 2 && automatic_bandwidth() && !bandwidth::probe_phase)
        bandwidth::initialize();
    refresh_started = query_phase = 0;
}

void destroy() {
    if (browser) {
        queued_lan = queued_master = queued_direct = 0;
        if (query_phase) {
            sdk::cancel(browser);
            query_phase = 3;
        }
        sdk::clear(browser);
        clear_visible_servers();
        sdk::destroy(browser);
        browser = nullptr;
    }
    if (automatic_bandwidth())
        bandwidth::stop();
    set_preferred_mode(Dword(mode()));
}

void tick() {
    sdk::pump(browser);
    if (automatic_bandwidth())
        bandwidth::poll();
    if (mode() == 1) {
        services::pending::tick_server_browser();
        services::pending::tick_internet_services();
    }
    if (!mode()) {
        if (!(lan_clock_guard & 1)) {
            lan_clock_guard |= 1;
            lan_clock = GetTickCount();
        }
        const Dword now = GetTickCount(), elapsed = now - lan_clock;
        const Dword sign = Dword(std::int32_t(elapsed) >> 31);
        if (std::int32_t((elapsed ^ sign) - sign) > 1000) {
            queued_lan = 1;
            lan_clock = now;
        }
    }
    if (query_phase)
        return;
    if (queued_direct) {
        const Dword result = sdk::refresh_server(browser, direct_address, int(direct_port), 1, 5);
        queued_direct = 0;
        direct_address[0] = 0;
        direct_port = 0;
        if (!result)
            query_phase = 1;
    } else if (queued_lan) {
        const Dword result = sdk::refresh_lan(browser, 1, services::dependency::lan_query_base,
                                             int(services::dependency::lan_query_base) + 10, 1);
        if (!result) {
            query_phase = 2;
            refresh_started = GetTickCount();
        }
        if (query_phase)
            queued_lan = 0;
    } else if (queued_master) {
        const Dword result = sdk::refresh_master(browser, 1, nullptr, 1);
        if (!result) {
            query_phase = 1;
            refresh_started = GetTickCount();
        }
        if (query_phase)
            queued_master = 0;
    }
}

void refresh(int kind) {
    if (kind == 1) {
        if (sdk::refresh_master(browser, 1, nullptr, 1))
            return;
    } else if (kind == 0)
        if (sdk::refresh_lan(browser, 1, services::dependency::lan_query_base,
                             int(services::dependency::lan_query_base) + 10, 1))
            return;
    query_phase = Dword(kind != 1) + 1;
    refresh_started = GetTickCount();
}

void cancel() {
    if (query_phase) {
        sdk::cancel(browser);
        query_phase = 3;
    }
}

void clear() {
    queued_lan = queued_master = queued_direct = 0;
    if (query_phase) {
        sdk::cancel(browser);
        query_phase = 3;
    }
    sdk::clear(browser);
    clear_visible_servers();
}

void set_mode(int kind) {
    packets::pending::session_kind = kind;
    queued_lan = queued_master = queued_direct = 0;
    if (query_phase) {
        sdk::cancel(browser);
        query_phase = 3;
    }
    sdk::clear(browser);
    clear_visible_servers();
    arm_mode();
}

void request_server(void *record) {
    if (query_phase == 2) {
        queued_lan = 0;
        sdk::cancel(browser);
        query_phase = 3;
    }
    queued_direct = 1;
    std::strcpy(direct_address, reinterpret_cast<char *>(bytes(record) + 24));
    direct_port = at<Dword>(record, 660);
}

void __cdecl browser_event(sdk::Browser *owner, Dword event, Dword, sdk::Server *entry, int) {
    if (event == 2) {
        if (!test_master && enforce_version && sdk::server_integer(entry, "nver", -1) != 18088178)
            return;
        const int kind = sdk::server_integer(entry, "pbmd", -1);
        if (!test_master && kind != mode())
            return;
        char address[500]{};
        std::strcpy(address, sdk::server_address_text(entry));
        const Dword port = sdk::server_integer(entry, "hostport", 0);
        const Dword query_port = sdk::server_port(entry);
        auto *screen = ui::multiplayer_screen;
        if (screen && ui::current_browser(screen)) {
            void *record =
                ui::find_browser_server(ui::current_browser(screen), address, port);
            const Byte existed = record != nullptr;
            if (!record) {
                if (Byte(ui::browser_has_room(ui::current_browser(screen)))) {
                    record = lifecycle::allocate(852, "GameSpy Server Entry", lifecycle::ui_pool);
                    if (record) {
                        try {
                            application::pending::construct_server_settings(record);
                        } catch (...) {
                            game::runtime::track::pending::release_named(
                                record, "GameSpy Server Entry", lifecycle::ui_pool);
                            throw;
                        }
                    }
                    std::strcpy(reinterpret_cast<char *>(bytes(record) + 24), address);
                    at<Dword>(record, 656) = port;
                    at<Dword>(record, 660) = query_port;
                    at<int>(record, 696) = kind;
                } else
                    cancel();
            }
            if (!record)
                return;
            read_server_basic(record, &entry);
            if (sdk::server_integer(entry, "laps", -1) != -1) {
                char text[500];
                std::strcpy(text, sdk::server_text(entry, "gamename", ""));
                std::strcpy(text, sdk::server_text(entry, "gamever", ""));
                read_server_rules(record, &entry);
                auto *details_screen = ui::multiplayer_screen;
                if (details_screen && ui::current_browser(details_screen))
                    ui::refresh_browser_details(ui::current_browser(details_screen), record);
            }
            at<Dword>(record, 700) = GetTickCount();
            ui::update_browser_server(ui::current_browser(screen), record, existed);
        } else
            cancel();
    } else if (event == 1 && !sdk::phase(owner)) {
        query_phase = 0;
        if (!mode()) {
            auto *screen = ui::multiplayer_screen;
            if (screen && ui::current_browser(screen))
                ui::remove_stale_browser_servers(ui::current_browser(screen), refresh_started);
        }
    }
}
} // namespace nfs::network::lobby
