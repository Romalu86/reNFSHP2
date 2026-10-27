#include "frontend_menu_pages.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "../../math/precision/extended_float.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
void set_slider_bar_position(SliderBar *, int);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using graphics::startup::display_caps;
using graphics::startup::settings;
using kernel::numeric::Extended;
static auto &selected_resolution = lifecycle::construction::unresolved::graphics_screen_resolution_setting;
static auto &selected_colour_depth = lifecycle::construction::unresolved::graphics_colour_depth_setting;
static Byte populating_modes;

static void desktop_dimensions(int *width, int *height, int *depth) {
    const HWND desktop = GetDesktopWindow();
    HDC dc = GetWindowDC(desktop);
    *width = GetDeviceCaps(dc, HORZRES);
    *height = GetDeviceCaps(dc, VERTRES);
    *depth = GetDeviceCaps(dc, BITSPIXEL);
    ReleaseDC(desktop, dc);
}
// Callers consume AL.
static bool resolution_limited_nvidia() {
    return display_caps && display_caps[516 / 4] == 282984480;
}
// Callers consume AL.
static bool resolution_limited_ati() {
    if (!display_caps)
        return false;
    const Dword adapter = display_caps[516 / 4];
    return adapter == 268587332 || adapter == 268587353 || adapter == 268586073;
}

static int maximum_detail(int kind) {
    if (kind == 0)
        return 4 - ((display_caps[760 / 4] & 1) != 0);
    if (kind == 1)
        return int((display_caps[584 / 4] & 131072 | 262144) >> 17);
    if (kind == 2)
        return 3 + (graphics::startup::stencil_enabled(&settings, 0) != 0);
    return 0;
}

static void apply_gamma(float *gamma_value) {
    const auto format = kernel::numeric::current_format();
    if (*gamma_value < 0.0f) {
        if (!graphics::gamma_saved)
            *gamma_value = 1.0f;
        else {
            WORD maximum = 0;
            for (unsigned index = 0; index < 256; ++index)
                if (maximum < graphics::gamma[0][index])
                    maximum = graphics::gamma[0][index];
            *gamma_value = float(Extended::integer(maximum, format) * 0.00001525902189314365386962890625f);
        }
    }
    WORD ramp[3][256];
    const Extended multiplier(*gamma_value, format), maximum(65535.0f, format);
    for (unsigned index = 0; index < 256; ++index) {
        auto sample = Extended::integer(WORD(index << 8), format) * multiplier;
        if (!(sample < maximum))
            sample = maximum;
        const WORD value = WORD(sample.truncate_to_integer());
        ramp[0][index] = ramp[1][index] = ramp[2][index] = value;
    }
    graphics::startup::set_gamma(&ramp[0][0]);
}
static void append_display_choice(ProfileToggle **toggle, const gui::String *text, Dword value) {
    family::ListItem item;
    construct_toggle_item(&item, text, value);
    try {
        invoke(&(*toggle)->items, 8, &item);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}

static void populate_resolutions(GraphicsPage *page) {
    const int depth = settings.race.colour_depth;
    const int current_width = graphics::startup::width(&settings, 0);
    const int current_height = graphics::startup::height(&settings, 0);
    if (!page->resolution)
        return;
    int desktop_width, desktop_height, desktop_depth;
    desktop_dimensions(&desktop_width, &desktop_height, &desktop_depth);
    auto *caps = display_caps;
    populating_modes = 1;
    invoke(&page->resolution->items, 1);
    for (int index = 0; index < 256; ++index) {
        const Dword *mode = caps + 768 / 4 + 6 * index;
        if (!mode[0])
            continue;
        const int width = int(mode[1]), height = int(mode[2]);
        if (!(width == current_width && height == current_height) && !(width > 639 && height > 479))
            continue;
        if (settings.limit_resolution) {
            if (resolution_limited_nvidia() || resolution_limited_ati())
                if (width > 1024 || height > 768)
                    continue;
            if (width > 1280 || height > 1024)
                continue;
        }
        if (!graphics::startup::fullscreen(&settings) &&
            !(width < desktop_width && height < desktop_height))
            continue;
        if (int(graphics::backend::format_depth(mode[5])) != depth)
            continue;
        pending::LocalFrontendString label("%d x %d");
        gui::string_format(&label.value, &label.value, width, height);
        append_display_choice(&page->resolution, &label.value, Dword(index));
    }
    select_toggle_value(page->resolution, Dword(settings.race.index));
    synchronize_page_toggle(page->resolution);
    populating_modes = 0;
}

static void populate_colour_depths(GraphicsPage *page) {
    if (!page->colour_depth)
        return;
    populating_modes = 1;
    invoke(&page->colour_depth->items, 1);
    if (!graphics::startup::fullscreen(&settings)) {
        int width, height, depth;
        desktop_dimensions(&width, &height, &depth);
        pending::LocalFrontendString label;
        pending::LocalFrontendString format;
        gui::string_reference(&format.value, 1558);
        gui::string_format(&label.value, &format.value, depth);
        append_display_choice(&page->colour_depth, &label.value, Dword(depth));
        settings.race.colour_depth = depth;
        select_toggle_value(page->colour_depth, Dword(depth));
        synchronize_page_toggle(page->colour_depth);
        page->colour_depth->button.base.base.flags |= 1;
        populating_modes = 0;
        return;
    }
    bool added16 = false, added_other = false;
    auto *caps = display_caps;
    for (int index = 0; index < 256; ++index) {
        const Dword *mode = caps + 768 / 4 + 6 * index;
        if (!mode[0])
            continue;
        const int width = int(mode[1]), height = int(mode[2]);
        if (!(width == settings.race.width && height == settings.race.height) &&
            !(width > 639 && height > 479))
            continue;
        const Dword depth = graphics::backend::format_depth(mode[5]);
        if (depth == 16) {
            if (added16)
                continue;
            added16 = true;
        } else {
            if (added_other)
                continue;
            added_other = true;
        }
        pending::LocalFrontendString label;
        gui::string_reference(&label.value, 1558);
        gui::string_format(&label.value, &label.value, depth);
        append_display_choice(&page->colour_depth, &label.value, depth);
        if (added16 && added_other)
            break;
    }
    select_toggle_value(page->colour_depth, Dword(settings.race.colour_depth));
    synchronize_page_toggle(page->colour_depth);
    if (page->colour_depth->items.grid.rows.count < 2)
        page->colour_depth->button.base.base.flags |= 1;
    else
        page->colour_depth->button.base.base.flags &= ~Dword(1);
    populating_modes = 0;
}
static family::SliderBar *find_graphics_slider(GraphicsPage *page, const char *name) {
    auto *child = static_cast<Widget *>(pending::find_child(page, name));
    return child && invoke<Dword>(child, 2, family::slider_bar_class_key)
               ? reinterpret_cast<family::SliderBar *>(child)
               : nullptr;
}

static void configure_slider_range(family::SliderBar *slider, int minimum, int maximum) {
    slider->minimum = minimum;
    slider->maximum = maximum <= minimum ? minimum : maximum;
    family::set_slider_bar_position(slider, slider->position);
}

void __fastcall initialize_graphics_page(GraphicsPage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->resolution = find_page_toggle(page, "<SCREEN_RES>");
    page->colour_depth = find_page_toggle(page, "<COLOR_DEPTH>");
    page->car_detail = find_graphics_slider(page, "<CAR_DETAIL>");
    page->track_detail = find_graphics_slider(page, "<TRACK_DETAIL>");
    page->effects = find_graphics_slider(page, "<SPECIAL_FX>");
    page->gamma = find_graphics_slider(page, "<BRITE>");
    if (page->gamma)
        set_enabled(&page->gamma->base.base, graphics::startup::fullscreen(&settings));
    if (page->colour_depth)
        set_enabled(&page->colour_depth->button.base.base,
                    !graphics::startup::fullscreen(&settings));
    if (page->car_detail)
        configure_slider_range(page->car_detail, 0, 4);
    if (page->track_detail)
        configure_slider_range(page->track_detail, 0, 3);
    if (page->effects)
        configure_slider_range(page->effects, 0, 4);
}

void __fastcall refresh_graphics_page(GraphicsPage *page, void *) {
    populate_resolutions(page);
    populate_colour_depths(page);
    if (page->gamma) {
        const auto format = kernel::numeric::current_format();
        const auto value = Extended(settings.gamma, format) * 100.0f;
        family::set_slider_bar_position(page->gamma, int(value.truncate_to_integer()));
    }
}

Dword __fastcall finish_graphics_page(GraphicsPage *page, void *, Dword result, Dword) {
    graphics::startup::write_render_settings();
    network::rules::pending::reload_lod(static_cast<Dword *>(system(140)));
    page->base.base.modal_result = result;
    return 1;
}

void __fastcall graphics_page_child_event(GraphicsPage *page, void *, Widget *sender, Dword event,
                                          Dword, Dword modifiers) {
    if (event == 9 || std::uint8_t(modifiers))
        return;
    if (event == 68 || event == 70) {
        if (populating_modes)
            return;
        if (!((page->resolution && reinterpret_cast<Widget *>(page->resolution) == sender) ||
              (page->colour_depth && reinterpret_cast<Widget *>(page->colour_depth) == sender)))
            return;
        if (int(selected_resolution) < 0 || int(selected_colour_depth) < 0)
            return;
        auto *caps = display_caps;
        settings.race.index = selected_resolution;
        graphics::backend::format_depth(display_caps[788 / 4 + 6 * selected_resolution]);
        const bool depth_changed = settings.frontend.colour_depth != int(selected_colour_depth);
        settings.race.colour_depth = settings.frontend.colour_depth = selected_colour_depth;
        settings.race.width = int(caps[772 / 4 + 6 * settings.race.index]);
        settings.race.height = int(caps[776 / 4 + 6 * settings.race.index]);
        if (depth_changed) {
            const int width = graphics::startup::width(&settings, 1),
                      height = graphics::startup::height(&settings, 1);
            pending::switch_display(1);
            gui::engine::Rectangle destination{gui::engine::rectangle_methods, 0, 0, width, height};
            gui::engine::configure_coordinates(reinterpret_cast<gui::engine::Rectangle *>(
                                                   game::runtime::bytes(system(96)) + 84),
                                               &destination);
            input::mouse_bounds(0, 0, 0, width, height, 255);
        }
        populate_resolutions(page);
        populate_colour_depths(page);
        graphics::startup::write_render_settings();
        network::rules::pending::reload_lod(static_cast<Dword *>(system(140)));
        return;
    }
    if (event != 90)
        return;
    if (page->gamma && reinterpret_cast<Widget *>(page->gamma) == sender) {
        const auto format = kernel::numeric::current_format();
        settings.gamma = float(Extended::integer(page->gamma->position, format) * 0.01f);
        apply_gamma(&settings.gamma);
        return;
    }
    family::SliderBar *slider = nullptr;
    int kind = 0, rule = 0;
    if (page->car_detail && reinterpret_cast<Widget *>(page->car_detail) == sender) {
        slider = page->car_detail;
        kind = 0;
        rule = 33;
    } else if (page->effects && reinterpret_cast<Widget *>(page->effects) == sender) {
        slider = page->effects;
        kind = 2;
        rule = 34;
    } else if (page->track_detail && reinterpret_cast<Widget *>(page->track_detail) == sender) {
        slider = page->track_detail;
        kind = 1;
        rule = 32;
    }
    if (!slider)
        return;
    const int maximum = maximum_detail(kind);
    if (maximum >= slider->position)
        return;
    game::audio::play_frontend_event(66, 63);
    family::set_slider_bar_position(slider, maximum);
    network::packets::pending::set_game_rule(system(72), rule, Dword(maximum));
    pending::MessageBox box;
    {
        pending::LocalFrontendString message;
        gui::string_reference(&message.value, 1805);
        pending::construct_message_box(&box, &message.value);
    }
    try {
        pending::run_message_box(&box, nullptr, 0);
    } catch (...) {
        pending::destroy_message_box(&box);
        throw;
    }
    pending::destroy_message_box(&box);
}
} // namespace nfs::application::frontend
