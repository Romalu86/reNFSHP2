#include "../flow/frontend_multiplayer.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
namespace nfs::services::dependency {
void set_recent_address(int, const char *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace {
int ask_password(Widget *page, gui::String *password) {
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string),
        title, caption(&lifecycle::construction::unresolved::shared_string);
    gui::string_reference(&title.value, 1578);
    return pending::prompt_text_entry(page, text.release(), title.release(), password,
                                      &lifecycle::construction::unresolved::shared_string,
                                      23, caption.release(), 0, nullptr, 0, 0);
}
void copy_password(const gui::String *password) {
    const char *text = gui::string_ascii(password);
    if (text && *text)
        std::strncpy(reinterpret_cast<char *>(bytes(system(112)) + 30), text, 24);
}
} // namespace

void connect_selected_server(BrowserView *page) {
    pending::LocalFrontendString password(
        &lifecycle::construction::unresolved::shared_string);
    void *record = page->selection;
    if (!record)
        return;
    char address[16];
    std::strncpy(address, reinterpret_cast<char *>(bytes(record) + 24), 16);
    const Dword port = at<Dword>(record, 656);
    if (at<std::uint8_t>(record, 693)) {
        if (ask_password(&page->base, &password.value) != 2)
            return;
        copy_password(&password.value);
    }
    for (;;) {
        services::pending::connect_client(system(112), address, port, reinterpret_cast<Dword>(page),
                                          0);
        if (at<Dword>(system(112), 68)) {
            finish_browser_view(page, 2);
            return;
        }
        if (at<Dword>(system(112), 56) != 2)
            return;
        const int result = ask_password(&page->base, &password.value);
        if (result != 2)
            return;
        copy_password(&password.value);
        if (result != 2 || at<Dword>(system(112), 68))
            return;
    }
}

void connect_private_server(PrivateView *page) {
    pending::LocalFrontendString address_text(invoke<const gui::String *>(page->address, 8));
    if (gui::string_equal(&address_text.value,
                          &lifecycle::construction::unresolved::shared_string))
        return;
    gui::string_make_narrow(&address_text.value);
    char address[16];
    std::strncpy(address, gui::string_ascii(&address_text.value), 16);
    address[15] = 0;
    pending::LocalFrontendString password(
        &lifecycle::construction::unresolved::shared_string);
    Dword quiet_password = 1;
    for (;;) {
        services::pending::connect_client(system(112), address, 0, reinterpret_cast<Dword>(page),
                                          quiet_password);
        quiet_password = 0;
        if (at<Dword>(system(112), 68)) {
            services::dependency::set_recent_address(0, address);
            finish_private_view(page, 2);
            return;
        }
        if (at<Dword>(system(112), 56) != 2)
            return;
        const int result = ask_password(&page->base, &password.value);
        if (result != 2)
            return;
        copy_password(&password.value);
        if (result != 2 || at<Dword>(system(112), 68))
            return;
    }
}
} // namespace nfs::application::frontend
