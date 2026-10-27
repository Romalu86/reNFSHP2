#include "hud_map.hpp"
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using pending::LocalFrontendString;

gui::String *localized_prefix(gui::String *output, Dword reference, const gui::String *suffix) {
    LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    gui::string_append(&text.value, suffix);
    return gui::string_construct_copy(output, &text.value);
}

void __fastcall tick_map(Map *map, void *, Dword ticks) {
    Dword visible = network::packets::rule_value(system(72), 45) != 0;
    const bool split = game::runtime::physics::split_screen() != 0;
    if (map_camera_hidden(0) && (!split || map_camera_hidden(1)))
        visible = 0;
    else if (visible) {
        const int elapsed = int(ticks) > 1 ? int(ticks) : 1;
        const Dword flags = (~map->base.base.flags >> 1) & 4294967041u;
        update_map_tracking(map, float(gui::ticks_to_seconds(elapsed)), Byte(flags));
    }
    pending::set_widget_visible(map, visible);
    if (!(map->style & 1))
        return;
    auto *label = static_cast<Widget *>(pending::find_child(map, "<TEXT>"));
    if (!label)
        return;
    void *race = at<void *>(system(92), 4);
    pending::set_widget_visible(label, 0);
    void *car = game::runtime::physics::local_car(system(68), 0);
    if (!car || !map_car_is_racer(car) || invoke<int>(race, 23) <= 1)
        return;
    const Dword slot = at<Dword>(car, 1772);
    if (!invoke<Byte>(race, 29, slot))
        return;
    const int player_position = invoke<int>(race, 42, slot);
    if (player_position <= 1)
        return;
    int count = at<int>(system(76), 84172);
    Dword offset = 84176;
    while (count > 0) {
        void *entity = at<void *>(system(76), offset);
        if (entity && at<void *>(entity, 368)) {
            const Dword other_slot = at<Dword>(at<void *>(entity, 368), 1772);
            const int position = invoke<int>(race, 42, other_slot);
            const char *name = reinterpret_cast<const char *>(game::runtime::bytes(entity) + 320);
            if (position == player_position - 1 && *name) {
                if (!network::packets::string_character(invoke<const gui::String *>(label, 8), 0) ||
                    label->value_84 != other_slot) {
                    label->value_84 = other_slot;
                    LocalFrontendString separator(": ");
                    gui::String prefix, complete;
                    localized_prefix(&prefix, Dword(position + 282), &separator.value);
                    try {
                        gui::append_ascii_value(&complete, &prefix, name);
                    } catch (...) {
                        gui::string_destruct(&prefix);
                        throw;
                    }
                    try {
                        invoke(label, 7, &complete);
                    } catch (...) {
                        gui::string_destruct(&complete);
                        gui::string_destruct(&prefix);
                        throw;
                    }
                    gui::string_destruct(&complete);
                    gui::string_destruct(&prefix);
                }
                pending::set_widget_visible(label, 1);
            }
        }
        offset += 4;
        --count;
    }
}
} // namespace nfs::application::frontend::hud
