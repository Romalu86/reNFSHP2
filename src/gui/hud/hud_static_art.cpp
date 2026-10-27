#include "hud_display.hpp"
#include <cmath>
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::system;
using kernel::numeric::Extended;

void update_static_art(StaticArt *widget) {
    void *car = binding_car(&widget->binding);
    if (!car || !at<Dword>(car, 7988)) {
        pending::set_widget_visible(widget, 0);
        return;
    }
    auto *systems = lifecycle::manager;
    void *race = at<void *>(systems->slots[92 / 4], 4);
    Dword visible = 0;
    switch (widget->display_type) {
    case 1:
        if (!at<Byte>(car, 776) && at<Byte>(car, 1584) > 1) {
            const auto format = kernel::numeric::current_format();
            const auto threshold =
                Extended(at<float>(car, 852 + Dword(at<Byte>(car, 1584)) * 4), format) * 0.9f;
            // TEST AH,5 / JP rejects greater, equal and unordered comparisons.
            visible = threshold < Extended(at<float>(car, 3168), format);
        }
        break;
    case 2:
    case 3:
    case 4:
        visible = pending::participant_is_cop(race, at<Dword>(car, 1772)) &&
                  !network::session_active();
        break;
    case 5:
        visible = network::packets::rule_value(
                      reinterpret_cast<Dword *>(systems->slots[72 / 4]), 44) != 0;
        break;
    case 6:
        visible = network::packets::rule_value(
                      reinterpret_cast<Dword *>(systems->slots[72 / 4]), 44) == 0;
        break;
    }
    pending::set_widget_visible(widget, visible);
}

Dword __fastcall static_art_event(StaticArt *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 104)
        update_static_art(widget);
    return gui::widgets::family::static_image_event(reinterpret_cast<Widget *>(widget), nullptr,
                                                    event, first, second);
}
} // namespace nfs::application::frontend::hud
