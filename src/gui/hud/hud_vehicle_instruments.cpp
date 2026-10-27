#include "hud_instruments.hpp"
#include <cstdio>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::system;
using kernel::numeric::Extended;

void __fastcall initialize_tachometer(Tachometer *gauge, void *) {
    gui::widgets::pending::request_general_layout(&gauge->base.base, nullptr);
    if (void *car = binding_car(&gauge->base.binding)) {
        for (int rpm = 5000; rpm < 12000; rpm += 500) {
            pending::LocalFrontendString name;
            gui::string_format_ascii(&name.value, "%d", rpm);
            if (void *label = pending::find_child(gauge, gui::string_ascii(&name.value)))
                pending::set_widget_visible(label, at<int>(car, 772) <= rpm);
        }
    }
}
 
static void read_tachometer_value(Tachometer *gauge, void *car) {
    const auto format = kernel::numeric::current_format();
    const auto range = Extended::integer(std::int32_t(Dword(gauge->maximum_rpm) - Dword(gauge->minimum_rpm)), format);
    if (!(range < Extended(0, format)) && double(range) != 0) {
        const auto rpm = Extended(at<float>(car, 4), format) * radians_per_second_to_rpm + .5f;
        const Dword rounded_rpm = Dword(rpm.truncate_to_integer());
        const auto value = Extended::integer(std::int32_t(rounded_rpm - Dword(gauge->minimum_rpm)), format) / range;
        set_needle_value(&gauge->base, float(value), 1);
    }
}

Dword __fastcall tachometer_event(Tachometer *gauge, void *, Dword event, Dword first, Dword second) {
    if (event == 104)
        if (void *car = binding_car(&gauge->base.binding)) read_tachometer_value(gauge, car);
    return gui::widgets::pending::general_event(&gauge->base.base, nullptr, event, first, second);
}

void __fastcall tick_tachometer(Tachometer *gauge, void *, Dword ticks) {
    tick_needle(&gauge->base, nullptr, ticks);
    void *server = system(108);
    Dword visible = 0;
    if (!at<Dword>(server, 44) || !at<Dword>(server, 48)) {
        visible = 1;
        if (void *camera = binding_camera(&gauge->base.binding)) {
            if (game::world::camera::top_mode(camera) == 3) {
                pending::LocalFrontendString type("HudDashboard");
                if (!find_menu_ancestor(&gauge->base.base.base, &type.value,
                        &lifecycle::construction::unresolved::shared_string, 0))
                    visible = 0;
            }
        }
    }
    pending::set_widget_visible(gauge, visible);
}

void update_tachometer(Tachometer *gauge) {
    if (void *car = binding_car(&gauge->base.binding)) read_tachometer_value(gauge, car);
}

Dword __fastcall load_tachometer(Tachometer *gauge, void *, void *configuration) {
    gui::engine::pending::read_int(configuration, "HudTachometer.mMinRPM", &gauge->minimum_rpm, 0);
    gui::engine::pending::read_int(configuration, "HudTachometer.mMaxRPM", &gauge->maximum_rpm, 0);
    return load_needle(&gauge->base, nullptr, configuration);
}

Dword __fastcall save_tachometer(Tachometer *gauge, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "HudTachometer.mMinRPM", gauge->minimum_rpm, 0);
    gui::engine::pending::write_int(configuration, "HudTachometer.mMaxRPM", gauge->maximum_rpm, 0);
    return save_needle(&gauge->base, nullptr, configuration);
}

Dword __fastcall speedometer_event(Speedometer *gauge, void *, Dword event, Dword first, Dword second) {
    if (event == 104) update_speedometer(gauge);
    return gui::widgets::pending::general_event(&gauge->base.base, nullptr, event, first, second);
}

void update_speedometer(Speedometer *gauge) {
    if (void *car = binding_car(&gauge->base.binding)) {
        const auto format = kernel::numeric::current_format();
        const Dword units = binding_rule(&gauge->base.binding, 44);
        const int minimum = units == 1 ? gauge->minimum_kph : gauge->minimum_mph;
        const int maximum = units == 1 ? gauge->maximum_kph : gauge->maximum_mph;
        const float range = float(Extended::integer(std::int32_t(Dword(maximum) - Dword(minimum)), format));
        if (range > 0) {
            const Dword current_units = binding_rule(&gauge->base.binding, 44);
            auto speed = Extended(at<float>(car, 3168), format);
            if (speed < Extended(0, format)) speed = -speed;
            const int first = current_units == 1 ? gauge->minimum_kph : gauge->minimum_mph;
            const auto value = (speed * (current_units == 1 ? metres_per_second_to_kph : metres_per_second_to_mph) -
                Extended::integer(first, format)) / Extended(range, format);
            set_needle_value(&gauge->base, float(value), 1);
        }
    }
}

Dword __fastcall load_speedometer(Speedometer *gauge, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_int(configuration, "HudSpeedometer.mMinMPH", &gauge->minimum_mph, 0);
    read_int(configuration, "HudSpeedometer.mMaxMPH", &gauge->maximum_mph, 0);
    read_int(configuration, "HudSpeedometer.mMinKPH", &gauge->minimum_kph, 0);
    read_int(configuration, "HudSpeedometer.mMaxKPH", &gauge->maximum_kph, 0);
    const auto format = kernel::numeric::current_format();
    if (!gauge->maximum_kph)
        gauge->maximum_kph = int((Extended::integer(gauge->maximum_mph, format) * mph_to_kph).truncate_to_integer());
    if (!gauge->minimum_kph)
        gauge->minimum_kph = int((Extended::integer(gauge->minimum_mph, format) * mph_to_kph).truncate_to_integer());
    return load_needle(&gauge->base, nullptr, configuration);
}

Dword __fastcall save_speedometer(Speedometer *gauge, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_int(configuration, "HudSpeedometer.mMinMPH", gauge->minimum_mph, 0);
    write_int(configuration, "HudSpeedometer.mMaxMPH", gauge->maximum_mph, 0);
    write_int(configuration, "HudSpeedometer.mMinKPH", gauge->minimum_kph, 0);
    write_int(configuration, "HudSpeedometer.mMaxKPH", gauge->maximum_kph, 0);
    return save_needle(&gauge->base, nullptr, configuration);
}
}
