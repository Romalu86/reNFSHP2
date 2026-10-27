#include "frontend_menu_controls.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

void __fastcall initialize_typewriter(TypeWriter *widget, void *) {
    gui::widgets::pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget),
                                                  nullptr);
    auto *wheel = static_cast<Widget *>(pending::find_child(widget, "<t0>"));
    widget->wheel = wheel && invoke<Dword>(wheel, 2, text_wheel_class_key)
                        ? reinterpret_cast<TextWheel *>(wheel)
                        : nullptr;
    invoke(widget, 7, &lifecycle::construction::unresolved::shared_string);
    widget->previous_button = family::named_child_id(reinterpret_cast<Widget *>(widget), "<bL>");
    widget->next_button = family::named_child_id(reinterpret_cast<Widget *>(widget), "<bR>");
}

static void insert_wheel_character(TypeWriter *widget) {
    pending::LocalFrontendString character;
    if (widget->wheel) {
        pending::LocalFrontendString text(invoke<const gui::String *>(widget->wheel, 8));
        gui::string_substring(&text.value, widget->wheel->selected_character, 1, &character.value);
    }
    family::insert_edit_character(reinterpret_cast<Widget *>(widget),
                                  static_cast<unsigned char>(*gui::string_ascii(&character.value)),
                                  0);
}

Dword __fastcall typewriter_event(TypeWriter *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 6) {
        if (first == 203 || first == 205) {
            if (widget->wheel)
                rotate_text_wheel(widget->wheel, first == 203 ? -1 : 1);
            latch_menu_spin(first == 203 ? widget->previous_button : widget->next_button);
            return 1;
        }
        if (first == 28) {
            if (!std::uint8_t(second))
                insert_wheel_character(widget);
            return 1;
        }
        if (first == 1) {
            if (!std::uint8_t(second))
                family::backspace_edit_character(reinterpret_cast<Widget *>(widget));
            return 1;
        }
    }
    return family::edit_text_event(reinterpret_cast<Widget *>(widget), nullptr, event, first,
                                   second);
}

void __fastcall draw_typewriter(TypeWriter *widget, void *, gui::engine::Engine *render) {
    gui::engine::Rectangle viewport = *gui::engine::current_viewport_bounds(&render->base);
    viewport.methods = gui::engine::rectangle_methods;
    if (widget->base.text_style & 8) {
        gui::engine::Rectangle clip;
        const auto *current = invoke<const gui::engine::Rectangle *>(render, 66);
        gui::engine::pending::intersect_rectangle(&clip, &viewport, current);
        invoke(render, 64, &clip);
        gui::engine::destroy_rectangle(&clip);
    }
    auto *base = reinterpret_cast<Widget *>(widget);
    family::text_draw(base, nullptr, reinterpret_cast<Dword>(render));
    if (active_widget_id == base->id && !invoke<Dword>(render, 59)) {
        gui::engine::Point caret;
        family::edit_position_to_screen(base, &caret, family::edit_caret(base));
        invoke(render, 14, inherited_font_style(base));
        invoke(render, 21, family::inherited_palette(base));
        pending::LocalFrontendString underline("__");
        const Dword colour = base->colour;
        invoke(render, 42, &underline.value, Dword(caret.x), Dword(caret.y), &colour, Dword(0));
        invoke(render, 42, &underline.value, Dword(caret.x), Dword(caret.y) - 1, &colour, Dword(0));
        gui::engine::destroy_point(&caret);
    }
    if (widget->base.text_style & 8)
        invoke(render, 65);
    gui::engine::destroy_rectangle(&viewport);
}

Dword __fastcall load_text_wheel(TextWheel *widget, void *, void *configuration) {
    gui::engine::pending::read_int(configuration, "HP2TypeWriterText.m_Spacing",
                                   &widget->character_pitch, 0);
    return family::text_load(reinterpret_cast<Widget *>(widget), nullptr,
                             reinterpret_cast<Dword>(configuration));
}

Dword __fastcall save_text_wheel(TextWheel *widget, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "HP2TypeWriterText.m_Spacing",
                                    widget->character_pitch, 0);
    return family::text_save(reinterpret_cast<Widget *>(widget), nullptr,
                             reinterpret_cast<Dword>(configuration));
}

void __fastcall update_text_wheel(TextWheel *widget, void *, Dword) {
    if (widget->scroll_offset)
        widget->scroll_offset /= 2;
}

void rotate_text_wheel(TextWheel *widget, int direction) {
    pending::LocalFrontendString text(invoke<const gui::String *>(widget, 8));
    int selected = int(Dword(widget->selected_character) + Dword(direction));
    widget->selected_character = selected;
    if (selected >= text.value.length)
        widget->selected_character = int(Dword(selected) - Dword(text.value.length));
    else if (selected < 0)
        widget->selected_character = int(Dword(selected) + Dword(text.value.length));
    widget->scroll_offset =
        int(Dword(widget->scroll_offset) + Dword(widget->character_pitch) * Dword(direction));
}

void copy_text_wheel_character(TextWheel *widget, gui::String *out) {
    pending::LocalFrontendString text(invoke<const gui::String *>(widget, 8));
    gui::string_substring(&text.value, widget->selected_character, 1, out);
}

void __fastcall draw_text_wheel(TextWheel *widget, void *, gui::engine::Engine *render) {
    gui::engine::Rectangle clip = *invoke<const gui::engine::Rectangle *>(render, 66);
    clip.methods = gui::engine::rectangle_methods;
    if (widget->text_style & 8) {
        gui::engine::intersect_in_place(&clip, gui::engine::current_viewport_bounds(&render->base));
        invoke(render, 64, &clip);
    }
    gui::engine::Rectangle viewport = *gui::engine::current_viewport_bounds(&render->base);
    viewport.methods = gui::engine::rectangle_methods;
    const Dword colour = widget->base.base.colour;
    invoke(render, 14, inherited_font_style(&widget->base.base));
    invoke(render, 21, family::inherited_palette(&widget->base.base));
    const auto *text = invoke<const gui::String *>(widget, 8);
    const int length = text->length, half = length / 2;
    const Dword centre = Dword(viewport.width / 2) + Dword(viewport.left);
    for (int index = 0; index < length; ++index) {
        int relative = int(Dword(index) - Dword(widget->selected_character));
        if (relative > half)
            relative = int(Dword(relative) - Dword(length));
        else if (relative < -half)
            relative = int(Dword(relative) + Dword(length));
        pending::LocalFrontendString character;
        gui::string_substring(text, index, 1, &character.value);
        gui::engine::Rectangle bounds;
        gui::engine::construct_rectangle(&bounds);
        invoke<Dword>(render, 15, &character.value, &bounds);
        const Dword x = Dword(widget->character_pitch) * Dword(relative) - Dword(bounds.width / 2) +
                       centre + Dword(widget->scroll_offset);
        invoke(render, 42, &character.value, x, Dword(viewport.top), &colour, Dword(0));
        gui::engine::destroy_rectangle(&bounds);
    }
    if (widget->text_style & 8)
        invoke(render, 65);
    gui::engine::destroy_rectangle(&viewport);
    gui::engine::destroy_rectangle(&clip);
}
} // namespace nfs::application::frontend
