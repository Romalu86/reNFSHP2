#include "frontend_scene_widgets.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include <cstdio>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;

void __fastcall initialize_menu_image_list(MenuImageList *list, void *) {
    auto *widget = reinterpret_cast<Widget *>(list);
    gui::widgets::pending::request_general_layout(&list->base.base, nullptr);
    list->left = family::named_child_id(widget, "<btl>");
    list->right = family::named_child_id(widget, "<btr>");
}

Dword __fastcall menu_image_list_key(MenuImageList *list, void *, Dword key, Dword modifiers) {
    if (!std::uint8_t(modifiers) && (key == 1 || key == 28 || key == 199 || key == 207))
        return 0;
    return family::image_list_key_down(reinterpret_cast<Widget *>(list), nullptr, key, modifiers);
}

Dword __fastcall menu_image_list_key_up(MenuImageList *list, void *, Dword key, Dword modifiers) {
    if (!std::uint8_t(modifiers) && (key == 1 || key == 28))
        return 0;
    return style_result_34(reinterpret_cast<Widget *>(list), nullptr, key, modifiers);
}

Dword __fastcall menu_image_list_event(MenuImageList *list, void *, Dword event, Dword first,
                                      Dword second) {
    if (event == 6 && (first == 207 || first == 199 ||
                       ((first == 203 || first == 205) && list->navigation_count <= 1 &&
                        !(list->base.style & 8))))
        return 1;
    return family::image_list_event(reinterpret_cast<Widget *>(list), nullptr, event, first,
                                    second);
}

void __fastcall menu_image_list_child(MenuImageList *list, void *, Widget *child, Dword event, Dword,
                                      Dword) {
    if (event != 43)
        return;
    if (child->id == list->left && (list->navigation_count > 1 || (list->base.style & 8)))
        invoke(list, 32, Dword(203), Dword(0));
    else if (child->id == list->right && (list->navigation_count > 1 || (list->base.style & 8)))
        invoke(list, 32, Dword(205), Dword(0));
}

static int image_cell_from_point(Widget *widget, const gui::engine::Point *point) {
    const int width = family::image_list(widget)->cell_size.x;
    if (!width)
        return 0;
    gui::engine::Rectangle bounds;
    client_bounds(widget, &bounds);
    int left = bounds.left, right = int(Dword(bounds.left) + Dword(bounds.width));
    if (right < left)
        left = right;
    if (left <= point->x && point->x <= right)
        right = point->x;
    const int first = -1 - widget->scroll.x / width;
    return (int(Dword(right) - Dword(width) * Dword(first) - Dword(widget->scroll.x))) / width + first;
}

void __fastcall menu_image_list_press(MenuImageList *list, void *, Dword button, Dword modifiers) {
    auto *widget = reinterpret_cast<Widget *>(list);
    if (!button && !std::uint8_t(modifiers)) {
        gui::engine::Point point;
        family::from_screen_point(widget, &point, gui::widgets::pending::pointer_position());
        const int cell = image_cell_from_point(widget, &point);
        gui::engine::destroy_point(&point);
        if (family::select_image_cell(widget, cell, 1))
            gui::events::post_widget(widget, 68, 0, 0);
    }
    family::image_list_pointer_press(widget, nullptr, button, modifiers);
}

void __fastcall menu_image_list_release(MenuImageList *list, void *, Dword first, Dword second) {
    style_result_38(reinterpret_cast<Widget *>(list), nullptr, first, second);
}

void __fastcall initialize_image_wheel(ImageWheel *wheel, void *) {
    gui::widgets::pending::request_general_layout(&wheel->base, nullptr);
    wheel->up = family::named_child_id(&wheel->base.base, "<bup>");
    wheel->down = family::named_child_id(&wheel->base.base, "<bdn>");
    auto *image = static_cast<Widget *>(pending::find_child(wheel, "<Dir>"));
    if (!image || !invoke<Dword>(image, 2, family::anim_image_class_key))
        image = nullptr;
    wheel->image = image;
    if (image) {
        wheel->count = game::runtime::at<int>(image, 340);
        family::select_sequence_frame(reinterpret_cast<gui::widgets::family::ImageSequence *>(
                                          game::runtime::bytes(image) + 328),
                                      0);
    }
}
static void select_wheel_frame(ImageWheel *wheel) {
    family::select_sequence_frame(
        reinterpret_cast<family::ImageSequence *>(game::runtime::bytes(wheel->image) + 328),
        wheel->selection);
}
static void move_image_wheel(ImageWheel *wheel, bool previous) {
    if (previous) {
        if (--wheel->selection < 0)
            wheel->selection = wheel->count - 2;
    } else if (++wheel->selection >= wheel->count - 1)
        wheel->selection = 0;
    select_wheel_frame(wheel);
}

Dword __fastcall image_wheel_event(ImageWheel *wheel, void *, Dword event, Dword first, Dword second) {
    if (event == 19)
        select_wheel_frame(wheel);
    else if (event == 107 || event == 106) {
        move_image_wheel(wheel, event == 107);
        latch_menu_spin(event == 107 ? wheel->up : wheel->down);
        return 1;
    }
    return gui::widgets::pending::general_event(&wheel->base, nullptr, event, first, second);
}

void __fastcall image_wheel_child(ImageWheel *wheel, void *, Widget *child, Dword event, Dword,
                                  Dword) {
    if (event != 43)
        return;
    if (child->id == wheel->up)
        move_image_wheel(wheel, true);
    else if (child->id == wheel->down)
        move_image_wheel(wheel, false);
}

Dword __fastcall image_wheel_key(ImageWheel *wheel, void *, Dword key, Dword modifiers) {
    if (!std::uint8_t(modifiers) && (key == 200 || key == 208)) {
        move_image_wheel(wheel, key == 200);
        return 1;
    }
    return gui::widgets::pending::general_key(&wheel->base, nullptr, key, modifiers);
}

void refresh_colour_wheel(ColourWheel *wheel) {
    const int count = wheel->count > 2 ? 3 : wheel->count;
    if (wheel->count == 1) {
        dispatch_target(wheel->swatches[1], 27, wheel->colours[0], 0);
        dispatch_target(wheel->swatches[1], 26, 1, 0);
        return;
    }
    for (int i = 0; i < count; ++i) {
        int row = (wheel->selected - 1 + i) % wheel->count;
        if (row < 0)
            row += wheel->count;
        dispatch_target(wheel->swatches[i], 27, wheel->colours[row], 0);
        dispatch_target(wheel->swatches[i], 26, 1, 0);
    }
}

void clear_colour_wheel(ColourWheel *wheel) {
    for (unsigned i = 0; i < 25; ++i) {
        wheel->colours[i] = 0;
        gui::string_copy(&wheel->names[i],
                         &lifecycle::construction::unresolved::shared_string);
    }
    for (Dword swatch : wheel->swatches)
        if (swatch) {
            dispatch_target(swatch, 27, 0, 0);
            dispatch_target(swatch, 26, 0, 0);
        }
    wheel->count = wheel->selected = 0;
}
// Owns the by-value label.
void append_colour_wheel(ColourWheel *wheel, gui::String label, Dword colour) {
    struct Cleanup {
        gui::String *text;
        ~Cleanup() {
            gui::string_destruct(text);
        }
    } cleanup{&label};
    wheel->colours[wheel->count] = colour;
    gui::string_copy(&wheel->names[wheel->count], &label);
    ++wheel->count;
    refresh_colour_wheel(wheel);
}

void colour_wheel_value(ColourWheel *wheel, Dword *value, int row) {
    row %= wheel->count;
    if (row < 0)
        row += wheel->count;
    *value = wheel->colours[row];
}

void __fastcall initialize_colour_wheel(ColourWheel *wheel, void *) {
    gui::widgets::pending::request_general_layout(&wheel->base, nullptr);
    wheel->up = family::named_child_id(&wheel->base.base, "<bup>");
    wheel->down = family::named_child_id(&wheel->base.base, "<bdn>");
    wheel->previous = family::named_child_id(&wheel->base.base, "<bprev>");
    wheel->next = family::named_child_id(&wheel->base.base, "<bnext>");
    for (unsigned i = 0; i < 3; ++i) {
        char name[80];
        std::sprintf(name, "<c%d>", i);
        wheel->swatches[i] = family::named_child_id(&wheel->base.base, name);
        dispatch_target(wheel->swatches[i], 27, 0, 0);
        dispatch_target(wheel->swatches[i], 26, 0, 0);
    }
    if (network::session_active())
        refresh_colour_wheel(wheel);
}
static void move_colour_wheel(ColourWheel *wheel, bool previous) {
    if (previous) {
        if (--wheel->selected < 0)
            wheel->selected = wheel->count - 1;
    } else if (++wheel->selected >= wheel->count)
        wheel->selected = 0;
    refresh_colour_wheel(wheel);
    gui::events::dispatch(wheel, 40, 0, 0);
}

Dword __fastcall colour_wheel_event(ColourWheel *wheel, void *, Dword event, Dword first,
                                   Dword second) {
    if (event != 107 && event != 106)
        return gui::widgets::pending::general_event(&wheel->base, nullptr, event, first, second);
    move_colour_wheel(wheel, event == 107);
    latch_menu_spin(event == 107 ? wheel->up : wheel->down);
    return 1;
}

void __fastcall colour_wheel_child(ColourWheel *wheel, void *, Widget *child, Dword event, Dword,
                                   Dword) {
    if (event != 43)
        return;
    if (child->id == wheel->up || child->id == wheel->previous) {
        move_colour_wheel(wheel, true);
        latch_menu_spin(wheel->up);
    } else if (child->id == wheel->down || child->id == wheel->next) {
        move_colour_wheel(wheel, false);
        latch_menu_spin(wheel->down);
    }
}

Dword __fastcall colour_wheel_key(ColourWheel *wheel, void *, Dword key, Dword modifiers) {
    if (!std::uint8_t(modifiers) && (key == 200 || key == 208)) {
        move_colour_wheel(wheel, key == 200);
        return 1;
    }
    return gui::widgets::pending::general_key(&wheel->base, nullptr, key, modifiers);
}
} // namespace nfs::application::frontend
