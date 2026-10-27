#include "network_lifecycle_runtime.hpp"
#include "../../frontend/screens/frontend_dialogs.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::network::session {

void show_connection_error(void *owner, void *, Dword joining) {
    packets::ScopedString text, title;
    gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
    gui::string_reference(&title.value, joining ? 1348 : 1703);
    Dword reference;
    switch (at<Dword>(owner, 56)) {
    case 1:
        reference = 1679;
        break;
    case 2:
        reference = 1721;
        break;
    case 3:
        reference = 1680;
        break;
    case 4:
        reference = 1681;
        break;
    case 5:
        reference = 1682;
        break;
    case 6:
        reference = 1683;
        break;
    case 8:
        reference = 1684;
        break;
    case 9:
        reference = 1685;
        break;
    case 10:
        reference = 1686;
        break;
    case 11:
        reference = 1687;
        break;
    case 12:
        reference = 1688;
        break;
    case 13:
        reference = 1689;
        break;
    case 14:
        reference = 1690;
        break;
    case 16:
        reference = 1692;
        break;
    case 17:
        reference = 1693;
        break;
    case 18:
        reference = 1694;
        break;
    case 19:
        reference = 1678;
        break;
    case 7:
    case 15:
        reference = 1691;
        break;
    default:
        reference = joining ? 1695 : 1691;
        break;
    }
    gui::string_reference(&text.value, reference);
    if (system(96)) {
        application::show_cursor(1);
        game::runtime::reset_requested = 1;
        application::pending::MessageBox box;
        application::pending::construct_message_box(&box, &text.value, &title.value);
        try {
            application::pending::run_message_box(&box, nullptr, 0);
        } catch (...) {
            application::pending::destroy_message_box(&box);
            throw;
        }
        game::runtime::reset_requested = 0;
        application::pending::destroy_message_box(&box);
    }
}

void reset_network_session(void *owner) {
    if (at<Dword>(client(), 68))
        at<Dword>(client(), 68) = 0;
    at<std::uint16_t>(owner, 8) = 0;
    at<std::uint16_t>(owner, 10) = 0;
    at<Dword>(owner, 48) = 0;
    at<Byte>(owner, 44) = 0;
    for (unsigned offset : {12u, 20u, 24u, 28u, 16u, 32u, 36u, 92u})
        at<Dword>(owner, offset) = 0;
    std::memset(bytes(owner) + 52, 0, 24);
    queue::clear(server_queue(static_cast<Dword *>(owner)));
    at<Dword>(owner, 80) = 0;
    game::runtime::reset_complete = 0;
    game::phase_reset = 0;
    game::runtime::reset_requested = 0;
    game::phase_pending = 0;
    game::phase_locked = 0;
    packets::pending::reset_lobby();
}
} // namespace nfs::network::session
