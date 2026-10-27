#include "hud_display.hpp"
#include "../../police/pursuit/chase_runtime.hpp"
#include "../../race/session/race_runtime.hpp"
#include <cmath>
#include <climits>
namespace nfs::network {
Dword race_pending(Dword *);
}
namespace nfs::application::pending {
void *participant_read_view(void *, Dword);
}
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using kernel::numeric::Extended;
using pending::LocalFrontendString;
Dword support_usage(const void *, int);

void refresh_text(Text *widget) {
    auto *base = &widget->base.base.base;
    if (!widget->binding.owner) {
        invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
        return;
    }
    void *car = binding_car(&widget->binding);
    if (!car || !at<Dword>(car, 7988)) {
        invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
        return;
    }
    void *race = at<void *>(system(92), 4);
    bool position_visible = invoke<int>(race, 23) >= 2 &&
                            !pending::participant_is_cop(race, at<Dword>(car, 1772));
    if (position_visible && network::session_active() && at<std::int16_t>(system(104), 8) == 6)
        position_visible = false;
    auto *participant = pending::participant(race, at<Dword>(car, 1772));
    LocalFrontendString text;
    auto integer = [&](Dword value) {
        gui::String number;
        pending::construct_integer_text(&number, int(value));
        try {
            gui::string_copy(&text.value, &number);
        } catch (...) {
            gui::string_destruct(&number);
            throw;
        }
        gui::string_destruct(&number);
    };
    const auto format = kernel::numeric::current_format();
    const Dword display_type = widget->display_type;
    switch (display_type) {
    case 1:
    case 2: {
        const Dword units = network::packets::rule_value(system(72), 44);
        if (units == 0 || units == 1) {
            float speed = at<float>(car, 3168);
            if (speed < 0)
                speed = -speed;
            const float stored_speed =
                float(Extended(speed, format) *
                      (units == 0 ? metres_per_second_to_mph : metres_per_second_to_kph));
            const auto rounded = Extended(stored_speed, format).round_to_integer();
            integer(rounded < INT_MIN || rounded > INT_MAX ? Dword(INT_MIN) : Dword(rounded));
            if (widget->display_type == 1) {
                gui::String separated, localized;
                gui::append_ascii_value(&separated, &text.value, " ");
                try {
                    append_localized_text(&localized, &separated, units == 0 ? 362 : 363);
                } catch (...) {
                    gui::string_destruct(&separated);
                    throw;
                }
                try {
                    gui::string_copy(&text.value, &localized);
                } catch (...) {
                    gui::string_destruct(&localized);
                    gui::string_destruct(&separated);
                    throw;
                }
                gui::string_destruct(&localized);
                gui::string_destruct(&separated);
            }
        }
        break;
    }
    case 3: {
        const Byte gear = at<Byte>(car, 1584);
        if (gear < 2)
            gui::string_reference(&text.value, gear == 0 ? 777 : 778);
        else
            integer(gear - 1);
        break;
    }
    case 4: {
        LocalFrontendString label;
        gui::string_reference(&label.value, 383);
        const Dword rpm =
            Dword((Extended(at<float>(car, 4), format) * radians_per_second_to_rpm + .5f)
                     .truncate_to_integer());
        gui::string_format(&text.value, &label.value, int(rpm) / 100);
        break;
    }
    case 5:
        if (position_visible &&
            !Byte(network::race_pending(reinterpret_cast<Dword *>(participant)))) {
            const int position = invoke<int>(race, 42, at<Dword>(car, 1772));
            if (position > 0) {
                integer(Dword(position));
                if (base->value_84 != Dword(position)) {
                    base->value_84 = Dword(position);
                    invoke(widget, 48);
                }
            }
        }
        break;
    case 6:
        if (position_visible &&
            !Byte(network::race_pending(reinterpret_cast<Dword *>(participant)))) {
            const Dword count = invoke<Dword>(race, 23);
            LocalFrontendString label;
            gui::string_reference(&label.value, 782);
            gui::string_format(&text.value, &label.value, count);
        }
        break;
    case 7:
        if (pending::bonus_display_enabled(race)) {
            auto *bonus_participant = pending::participant(race, at<Dword>(car, 1772));
            Dword points = 0;
            for (Dword kind = 0; kind < 5; ++kind)
                points += pending::participant_award(bonus_participant, kind);
            if (base->value_84 != points) {
                base->value_84 = points;
                if (points)
                    invoke(widget, 48);
            }
            gui::string_format_ascii(&text.value, "%d ", points);
            gui::string_append_reference(&text.value, 465);
        }
        break;
    case 8:
    case 9:
    case 10:
        write_mode_text(widget, display_type - 8, car, &text.value, nullptr);
        break;
    case 11:
    case 12:
    case 13:
        write_mode_text(widget, display_type - 11, car, nullptr, &text.value);
        break;
    case 14:
    case 15:
    case 16: {
        const Dword slot = at<Dword>(car, 1772);
        if (pending::participant_is_cop(race, slot)) {
            const int equipment = display_type == 14 ? 8 : display_type == 15 ? 7 : 3;
            void *rules = system(72);
            const Dword used = support_usage(pending::participant_read_view(race, slot), equipment);
            integer(game::runtime::ai::lifecycle::pending::support_statistic_limit(
                        rules, Dword(equipment)) -
                    used);
        }
        break;
    }
    }
    if (!network::packets::string_character(&text.value, 0))
        pending::set_widget_visible(widget, 0);
    else {
        pending::set_widget_visible(widget, 1);
        invoke(widget, 7, &text.value);
    }
}

void __fastcall initialize_text(Text *widget, void *) {
    initialize_flash_text(&widget->base, nullptr);
    invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
    widget->base.base.base.value_84 = 0;
}

void __fastcall begin_text(Text *widget, void *) {
    if (network::packets::string_character(invoke<const gui::String *>(widget, 8), 0) &&
        game::runtime::race_started)
        begin_flash_text(&widget->base, nullptr);
}

Dword __fastcall text_event(Text *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 104)
        refresh_text(widget);
    return flash_text_event(&widget->base, nullptr, event, first, second);
}
} // namespace nfs::application::frontend::hud
