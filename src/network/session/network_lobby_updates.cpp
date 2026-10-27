#include "../../frontend/flow/frontend_multiplayer.hpp"
#include "../gamespy/gamespy_updates.hpp"
#include "nfs/core/application/application.hpp"
#include <shellapi.h>
#include <cstdlib>
#include <cstring>
namespace nfs::network::lobby {
Byte notify_no_update{}, fallback_lan{};
char update_url[2000]{};
namespace {
Byte copied_update_character{};
}
using namespace application::frontend;
using game::runtime::at;
using game::runtime::system;

static void __cdecl launch_update_url() {
    char suffix[10]{};
    const auto length = std::strlen(update_url);
    if (length > 3) {
        std::strcpy(suffix, update_url + length - 4);
        _strlwr(suffix);
    }
    if (!std::strcmp(suffix, ".exe"))
        ShellExecuteA(nullptr, nullptr, "fpupdate.exe", update_url, "", 5);
    else
        ShellExecuteA(nullptr, nullptr, update_url, nullptr, "", 5);
}

static void __cdecl version_checked(Dword available, Dword, const char *name, Dword file_id,
                                    const char *url, Dword) {
    namespace pending = application::pending;
    session::internet_query_active = 0;
    if (!available) {
        check_version = 0;
        if (notify_no_update) {
            pending::LocalFrontendString text;
            gui::string_reference(&text.value, 1548);
            application::show_cursor(1);
            const Byte reset = game::runtime::reset_requested;
            game::runtime::reset_requested = 1;
            pending::MessageBox box;
            pending::construct_message_box(&box, &text.value);
            try {
                pending::run_message_box(&box, nullptr, 0);
                game::runtime::reset_requested = reset;
            } catch (...) {
                pending::destroy_message_box(&box);
                throw;
            }
            pending::destroy_message_box(&box);
        }
        return;
    }
    pending::LocalFrontendString version(name), text;
    gui::string_make_wide(&version.value);
    {
        pending::LocalFrontendString format;
        gui::string_reference(&format.value, 1546);
        gui::string_format(&text.value, &format.value, gui::string_data(&version.value));
    }
    application::show_cursor(1);
    const Byte reset = game::runtime::reset_requested;
    game::runtime::reset_requested = 1;
    pending::MessageBox box;
    pending::construct_message_box(&box, &text.value, 66);
    try {
        const int result = pending::run_message_box(&box, nullptr, 0);
        game::runtime::reset_requested = reset;
        if (result == 2) {
            std::strcpy(update_url, url);
            if (!*url)
                std::sprintf(update_url, "http://www.fileplanet.com/index.asp?file=%d", file_id);
            std::atexit(launch_update_url);
            if (!system(96) || !at<void *>(system(96), 40))
                std::exit(1);
            gui::widgets::family::finish_modal(at<Widget *>(system(96), 40), 6, 0);
            pending::clear_screen_queue(system(96));
        } else if (multiplayer_screen && fallback_lan)
            select_multiplayer_mode(multiplayer_screen, 0);
    } catch (...) {
        pending::destroy_message_box(&box);
        throw;
    }
    pending::destroy_message_box(&box);
}

void check_for_update(Byte notify, Byte fallback) {
    notify_no_update = notify;
    fallback_lan = fallback;
    if (!session::internet_query_active) {
        copied_update_character = 0;
        update_url[0] = copied_update_character;
        session::internet_query_active = 1;
        session::pending::updates::request_version(10128, session::version_text, 661,
                                                   version_checked, 0, 0);
    }
}

void send_motd() {
    session::pending::updates::request_motd(0, 10128, session::version_text, 661, 0);
    session::browser_started = GetTickCount();
    request_motd = 0;
}
} // namespace nfs::network::lobby
