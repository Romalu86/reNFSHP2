#include "../screens/frontend_menu_roots.hpp"
namespace nfs::gui::input {
const void *language_text(int);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

void show_car_statistics(CarSelection *screen, std::uint8_t show) {
    auto *label = track_child(screen, "<DESCRIBE_CAR>", family::text_class_key);
    if (!label)
        return;
    pending::set_widget_visible(label, show == 0);
    if (screen->statistics)
        pending::set_widget_visible(screen->statistics, show);
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    const int selection = screen->selection;
    if (selection > 37 && selection < 62)
        gui::string_append_ascii(&text.value, "NFS");
    gui::string_append_reference(&text.value, 733);
    const char *resource = gui::string_ascii(&screen->descriptions[screen->car]);
    void *catalogue = bytes(screen->vehicles) + 4;
    const Dword make = catalogue_resource_make(catalogue, resource);
    if (make != 97) {
        gui::string_append_reference(&text.value, make);
        gui::string_append_reference(&text.value, 733);
    }
    gui::string_append_reference(
        &text.value,
        catalogue_resource_name(catalogue, gui::string_ascii(&screen->descriptions[screen->car])));
    if (show) {
        if (!screen->preview)
            return;
        gui::string_append_reference(&text.value, 722);
        gui::string_append_reference(&text.value, 733);
        gui::string_append_reference(&text.value, 733);
        const Dword power = catalogue_specification(catalogue, screen->car, 0);
        if (power) {
            gui::string_append_reference(
                &text.value, network::packets::rule_value(system(72), 44) ? 113 : 112);
            pending::LocalFrontendString power_text;
            gui::string_reference(&power_text.value, power);
            gui::string_format(&text.value, &text.value, gui::string_ascii(&power_text.value));
            gui::string_append_reference(&text.value, 733);
            gui::string_append_reference(&text.value, 733);
        }
        const bool metric = network::packets::rule_value(system(72), 44) != 0;
        const Dword speed = catalogue_specification(catalogue, screen->car, metric ? 2 : 1);
        if (speed) {
            gui::string_append_reference(&text.value, metric ? 148 : 147);
            gui::string_format(&text.value, &text.value, gui::input::language_text(int(speed)));
            gui::string_append_reference(&text.value, 733);
            gui::string_append_reference(&text.value, 733);
        }
        const Dword acceleration = catalogue_specification(catalogue, screen->car, 3);
        if (acceleration) {
            gui::string_append_reference(&text.value, 209);
            gui::string_format(&text.value, &text.value,
                               gui::input::language_text(int(acceleration)));
        }
        clear_ticker_messages(screen->statistics);
        for (unsigned i = 0; i < 4; ++i)
            queue_ticker_message(screen->statistics, &text.value, 1,
                                 &screen->statistics->base.base.colour, 0, 100, 0);
        return;
    }
    if (selection == 78 || selection == 16)
        gui::string_reference(&text.value, 2077);
    else if (selection == 54)
        gui::string_reference(&text.value, 2078);
    if (selection == 70 || selection == 8)
        gui::string_reference(&text.value, 2079);
    else if (selection == 46)
        gui::string_reference(&text.value, 2080);
    label->colour = car_unlocked(screen) ? Dword(-1) : 2952790015u;
    gui::engine::Point before, after;
    menu_rectangle_center(&label->bounds, &before);
    invoke<void>(label, 7, &text.value);
    invoke<void>(label, 6, Dword(64));
    menu_rectangle_center(&label->bounds, &after);
    label->bounds.left += before.x - after.x;
    label->bounds.top += before.y - after.y;
    after.methods = gui::base_methods;
    before.methods = gui::base_methods;
}
} // namespace nfs::application::frontend
