#include "../screens/frontend_menu_roots.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
static const Dword selection_order[72] = {
    7,  9,  21, 17, 18, 3,  22, 14, 23, 13, 5,  6,  20, 15, 0,  19, 1,  8,
    2,  16, 4,  12, 10, 11, 45, 47, 59, 55, 56, 41, 60, 52, 61, 51, 43, 44,
    58, 53, 38, 57, 39, 46, 40, 54, 42, 50, 48, 49, 69, 71, 83, 79, 80, 65,
    84, 76, 85, 75, 67, 68, 82, 77, 62, 81, 63, 70, 64, 78, 66, 74, 72, 73};

gui::String *car_logo_name(gui::String *output, Dword car) {
    static const char *const logo_names[] = {"pors", "ferr", "pors", "merc", "merc", "ford",
                                             "chev", "ford", "lamb", "lotu", "mcla", "mcla",
                                             "ferr", "hsvx", "bmwx", "ferr", "lamb", "opel",
                                             "ford", "asto", "dodg", "vaux", "jagu", "bmwx"};
    return gui::string_construct_ascii(output, car < 24 ? logo_names[car] : "ferr");
}

gui::String *car_frame_name(gui::String *output, Dword car) {
    static const char *const frame_names[] = {"911t", "barc", "carr", "cl55", "clkg", "cobr",
                                              "corv", "crnv", "diab", "elis", "mcf1", "f1lm",
                                              "f50x", "hsvc", "m5xx", "mode", "murc", "spee",
                                              "ts50", "vanq", "vipr", "vx22", "jxkr", "bmz8"};
    return gui::string_construct_ascii(output, car < 24 ? frame_names[car] : "asto");
}

void populate_car_selection(CarSelection *screen) {
    if (!screen->car_list)
        return;
    auto *widget = reinterpret_cast<Widget *>(screen->car_list);
    pending::LocalFrontendString logo;
    void *catalogue = bytes(screen->vehicles) + 4;
    const bool special = std::uint8_t(services::special_race_mode(system(72))) != 0;
    screen->car_list->base.style = special ? 0 : 8;
    int preferred = -1, fallback = -1, fallback_car = -1;
    for (unsigned i = 0; i < (special ? 24u : 72u); ++i) {
        const int choice = int(selection_order[i]);
        int car = choice >= 86   ? 0
                  : choice >= 62 ? choice - 62
                  : choice >= 38 ? choice - 38
                                 : choice;
        int selection = choice;
        if (special) {
            if (screen->police && !at<std::uint8_t>(screen->vehicles, Dword(car) * 1228 + 1084))
                continue;
            if (!screen->police && (car == 7 || car == 45))
                continue;
            if (screen->police && car != 7 && car != 45)
                selection = car + 62;
        } else {
            if (choice >= 24 && choice <= 37)
                continue;
            if (!screen->police) {
                if (choice >= 62 || choice == 7 || choice == 45)
                    continue;
                if (choice > 37 && !at<std::uint8_t>(screen->vehicles, Dword(car) * 1228 + 1032))
                    continue;
            } else if (choice <= 61 ||
                       !at<std::uint8_t>(screen->vehicles, Dword(car) * 1228 + 1084))
                continue;
        }
        gui::String next_logo;
        car_logo_name(&next_logo, Dword(car));
        try {
            gui::string_copy(&logo.value, &next_logo);
        } catch (...) {
            gui::string_destruct(&next_logo);
            throw;
        }
        gui::string_destruct(&next_logo);
        const std::uint8_t enabled =
            special ? catalogue_entry_enabled(catalogue, car)
                    : at<std::uint8_t>(screen->preferences, 7328 + Dword(choice));
        const Dword item = family::append_image_frame_name(widget, gui::string_ascii(&logo.value),
                                                          Dword(selection), enabled);
        if (!item || preferred >= 0 || (special && !enabled))
            continue;
        const Dword saved = at<Dword>(
            screen->preferences,
            special ? (network::packets::rule_value(system(72), 1) == 4 ? 7524 : 7528)
            : screen->police ? 7520
                             : 7516);
        if (Dword(selection) == saved) {
            preferred = menu_image_index(widget, item);
            screen->selection = selection;
        } else if (enabled) {
            fallback = menu_image_index(widget, item);
            fallback_car = selection;
        }
    }
    int selected = preferred;
    if (preferred < 0) {
        family::select_image_cell(widget, fallback, 0);
        screen->car = fallback_car >= 86   ? 0
                      : fallback_car >= 62 ? fallback_car - 62
                      : fallback_car >= 38 ? fallback_car - 38
                                           : fallback_car;
        selected = fallback;
    } else {
        family::select_image_cell(widget, preferred, 0);
        const int car = screen->selection;
        screen->car = car >= 86 ? 0 : car >= 62 ? car - 62 : car >= 38 ? car - 38 : car;
    }
    (void)selected;
    align_menu_image_list(widget);
    refresh_menu_image_navigation(screen->car_list);
}
} // namespace nfs::application::frontend
