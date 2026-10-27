#include "gui_widget_interaction.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void __fastcall slider_bar_prepare_draw(Widget *widget, void *, Dword render_address) {
    auto *slider = reinterpret_cast<SliderBar *>(widget);
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    Rectangle client;
    client_bounds(widget, &client);
    if (Widget *title = find_child(widget, slider->title)) {
        invoke(title, 7, invoke<const String *>(widget, 8));
        invoke(render, 21, inherited_palette(widget));
        Dword colour;
        const Dword palette = widget->flags & 1 ? 2 : active_widget_id == widget->id ? 1 : 0;
        title->colour = *invoke<Dword *>(render, 23, &colour, palette);
    }
    slider->track.top = 0;
    slider->track.height = widget->bounds.height;
    Widget *minus = find_child(widget, slider->minus);
    slider->track.left = minus ? int(Dword(minus->bounds.left) + Dword(minus->bounds.width)) : 0;
    Widget *plus = find_child(widget, slider->plus);
    slider->track.width =
        int(Dword(plus ? plus->bounds.left : widget->bounds.width) - Dword(slider->track.left));
    if (Widget *thumb = find_child(widget, slider->thumb)) {
        int center = slider->track.left;
        if (slider->minimum < slider->maximum)
            center = int(Dword(center) + Dword(int((Dword(slider->position) - Dword(slider->minimum)) *
                                                 Dword(slider->track.width)) /
                                             int(Dword(slider->maximum) - Dword(slider->minimum))));
        thumb->bounds.left = int(Dword(center) - Dword(thumb->bounds.width / 2));
    }
    prepare_draw(widget, nullptr, render);
    engine::destroy_rectangle(&client);
}

void __fastcall slider_bar_initialize(Widget *widget, void *) {
    auto *slider = reinterpret_cast<SliderBar *>(widget);
    if (!(slider->style & 1)) {
        slider->minus = named_child_id(widget, "<MINUS>");
        slider->plus = named_child_id(widget, "<PLUS>");
        slider->fill = named_child_id(widget, "<FILL>");
        slider->thumb = named_child_id(widget, "<TAB>");
        slider->title = named_child_id(widget, "<TITLE>");
    } else {
        const int size = scale_gui_distance(16);
        Rectangle client;
        client_bounds(widget, &client);
        set_widget_resource(widget, &lifecycle::construction::unresolved::shared_string);
        auto *background = static_cast<Widget *>(create_image_box(widget));
        Rectangle square{engine::rectangle_methods, 0, 0, size, size};
        engine::copy_rectangle_fields(&background->bounds, &square);
        background->flags = 1;
        background->layout_flags = 128;
        background->colour = 4288651167u;
        set_z_order(background, 4);
        at<Dword>(background, 268) = 4;
        initialize_tree(background);
        auto *thumb = static_cast<Widget *>(create_image_box(widget));
        Rectangle thumb_bounds{engine::rectangle_methods, 0, 0, size / 2, client.height};
        engine::copy_rectangle_fields(&thumb->bounds, &thumb_bounds);
        thumb->flags = 4704;
        thumb->colour = 4289703855u;
        set_z_order(thumb, 3);
        at<Dword>(thumb, 268) = 8;
        thumb->layout_flags = 1024;
        initialize_tree(thumb);
        slider->thumb = thumb->id;
        auto *minus = static_cast<Widget *>(create_button(widget));
        Rectangle button_bounds{engine::rectangle_methods, 0, 0, size, client.height};
        engine::copy_rectangle_fields(&minus->bounds, &button_bounds);
        minus->colour = 4288651167u;
        minus->flags = 4704;
        set_z_order(minus, 2);
        reinterpret_cast<Button *>(minus)->style = 9;
        minus->layout_flags = 1026;
        initialize_tree(minus);
        slider->minus = minus->id;
        auto *plus = static_cast<Widget *>(create_button(widget));
        plus->bounds.height = client.height;
        plus->bounds.left = int(Dword(client.width) - Dword(size));
        plus->bounds.top = 0;
        plus->bounds.width = size;
        plus->colour = 4288651167u;
        plus->flags = 4704;
        set_z_order(plus, 1);
        reinterpret_cast<Button *>(plus)->style = 9;
        plus->layout_flags = 1025;
        initialize_tree(plus);
        slider->plus = plus->id;
        engine::destroy_rectangle(&client);
    }
    pending::request_general_layout(reinterpret_cast<GeneralWidget *>(widget), nullptr);
}

void __fastcall slider_bar_pointer_move(Widget *widget, void *, Dword first, Dword modifiers) {
    auto *slider = reinterpret_cast<SliderBar *>(widget);
    if (application::pending::captured_widget_id == widget->id && (modifiers & 4194304)) {
        Point local;
        from_screen_point(widget, &local, pending::pointer_position());
        const int value = int(int((Dword(local.x) - Dword(slider->track.left)) *
                                  (Dword(slider->maximum) - Dword(slider->minimum))) /
                                  slider->track.width +
                              Dword(slider->minimum));
        slider->position = clamp_gui_integer(value, slider->minimum, slider->maximum);
        events::post_widget(widget, 90, 0, 0);
        engine::destroy_point(&local);
    }
    pending::update_active_style(widget, nullptr, first, modifiers);
}

Dword __fastcall slider_bar_pointer_press(Widget *widget, void *, Dword button, Dword modifiers) {
    auto *slider = reinterpret_cast<SliderBar *>(widget);
    if (!button) {
        Point local;
        from_screen_point(widget, &local, pending::pointer_position());
        if (!std::uint8_t(modifiers) && contains_point(&slider->track, &local))
            capture_widget(widget);
        if (application::pending::captured_widget_id == widget->id && slider->track.width) {
            const int value = int(int((Dword(local.x) - Dword(slider->track.left)) *
                                      (Dword(slider->maximum) - Dword(slider->minimum))) /
                                      slider->track.width +
                                  Dword(slider->minimum));
            slider->position = clamp_gui_integer(value, slider->minimum, slider->maximum);
            events::post_widget(widget, 90, 0, 0);
            activate(widget);
            engine::destroy_point(&local);
            return 1;
        }
        engine::destroy_point(&local);
    }
    return pending::update_focus_style(widget, nullptr, button, modifiers);
}
// Four stack words; the last is unused.
void __fastcall slider_bar_configure_geometry(Widget *widget, void *, Dword sender_address,
                                              Dword event, Dword state, Dword) {
    auto *slider = reinterpret_cast<SliderBar *>(widget);
    if (event != 43)
        return;
    const Dword sender = reinterpret_cast<Widget *>(sender_address)->id;
    if (sender == slider->minus) {
        slider->position = clamp_gui_integer(int(Dword(slider->position) - Dword(slider->step)),
                                             slider->minimum, slider->maximum);
        events::post_widget(widget, 90, state, 0);
        activate(widget);
    }
    if (sender == slider->plus) {
        slider->position = clamp_gui_integer(int(Dword(slider->position) + Dword(slider->step)),
                                             slider->minimum, slider->maximum);
        events::post_widget(widget, 90, state, 0);
        activate(widget);
    }
}
} // namespace nfs::gui::widgets::family
