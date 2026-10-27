#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using kernel::numeric::Extended;
static family::SliderBar *deadzone_slider(DeadzoneDialog *dialog, int index) {
    pending::LocalFrontendString name;
    gui::string_format_ascii(&name.value, "<AXIS_%d>", index);
    auto *child =
        static_cast<Widget *>(pending::find_child(dialog, gui::string_ascii(&name.value)));
    return child && invoke<Dword>(child, 2, family::slider_bar_class_key)
               ? reinterpret_cast<family::SliderBar *>(child)
               : nullptr;
}

void __fastcall initialize_deadzone_dialog(DeadzoneDialog *dialog, void *) {
    family::dialog_initialize(&dialog->base, nullptr);
    for (int index = 0; index < 26; ++index) {
        auto *slider = deadzone_slider(dialog, index);
        if (!slider) {
            dialog->axis_count = 0;
            return;
        }
        set_enabled(&slider->base.base, 0);
        slider->base.base.value_84 = ~Dword(0);
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 758);
        invoke(slider, 7, &text.value);
    }
    dialog->axis_count = 0;
}

static family::SliderBar *add_deadzone_axis(DeadzoneDialog *dialog, Dword binding_index,
                                            const gui::String *text, int position) {
    auto *slider = deadzone_slider(dialog, int(dialog->axis_count));
    if (slider) {
        ++dialog->axis_count;
        slider->base.base.value_84 = binding_index;
        set_enabled(&slider->base.base, 1);
        pending::set_widget_visible(slider, 1);
        invoke(slider, 7, text);
        family::set_slider_bar_position(slider, position);
    }
    return slider;
}

static family::SliderBar *find_deadzone_axis(DeadzoneDialog *dialog, Dword binding_index) {
    for (Dword index = 0; index < dialog->axis_count; ++index) {
        auto *slider = deadzone_slider(dialog, int(index));
        if (!slider)
            return nullptr;
        if (slider->base.base.value_84 == binding_index)
            return slider;
    }
    return nullptr;
}
static void *deadzone_record(void *records, Dword binding) {
    const Dword axis = binding >> 24 & 15, minimum = binding >> 16 & 255,
               maximum = binding >> 8 & 255;
    for (unsigned index = 0; index < 32; ++index) {
        void *record = bytes(records) + index * 16;
        if (at<Dword>(record, 0) == axis && at<Dword>(record, 4) == minimum &&
            at<Dword>(record, 8) == maximum)
            return record;
    }
    return nullptr;
}

static void set_deadzone_record(void *records, int axis, int minimum, int maximum, int distance) {
    if (axis == -1)
        return;
    void *free_record = nullptr;
    for (unsigned index = 0; index < 32; ++index) {
        void *record = bytes(records) + index * 16;
        if (at<int>(record, 0) == axis && at<int>(record, 4) == minimum &&
            at<int>(record, 8) == maximum) {
            at<int>(record, 12) = distance;
            return;
        }
        if (!free_record && at<int>(record, 0) == -1)
            free_record = record;
    }
    if (free_record) {
        at<int>(free_record, 0) = axis;
        at<int>(free_record, 4) = minimum;
        at<int>(free_record, 8) = maximum;
        at<int>(free_record, 12) = distance;
        ++at<Dword>(records, 512);
    }
}

Dword run_deadzone_dialog(DeadzoneDialog *dialog, Dword *bindings, int count) {
    void *list = controllers::get_bindings();
    void *records = bytes(system(72)) + 52060;
    const auto format = kernel::numeric::current_format();
    for (int index = 0; index < count; ++index) {
        void *control = resolve_menu_control(list, bindings + index, 0, false);
        if (!control || std::uint8_t(bindings[index]) != 1)
            continue;
        void *axis = invoke<void *>(control, 3);
        if (!axis)
            continue;
        void *record = deadzone_record(records, bindings[index]);
        {
            pending::LocalFrontendString label;
            gui::string_reference(&label.value, at<Dword>(axis, 8));
        }
        int position = 0;
        if (record && at<int>(record, 12) > 0)
            position =
                int((Extended::integer(at<int>(record, 12), format) * 0.0039215688593685626983642578125f * 100.0f)
                        .truncate_to_integer());
        pending::LocalFrontendString label;
        gui::string_reference(&label.value, at<Dword>(axis, 8));
        add_deadzone_axis(dialog, Dword(index), &label.value, position);
    }
    const Dword accepted = gui::run_modal(dialog, nullptr) == 2;
    if (accepted) {
        for (int index = 0; index < count; ++index) {
            void *control = resolve_menu_control(list, bindings + index, 0, false);
            if (!control || std::uint8_t(bindings[index]) != 1)
                continue;
            void *axis = invoke<void *>(control, 3);
            if (!axis)
                continue;
            auto *slider = find_deadzone_axis(dialog, Dword(index));
            if (!slider)
                continue;
            int position = slider->position;
            if (position < 0)
                position = 0;
            if (position > 255)
                position = 255;
            const int distance =
                int((Extended::integer(position, format) * 0.01f * 255.0f).truncate_to_integer());
            pending::LocalFrontendString label;
            gui::string_reference(&label.value, at<Dword>(axis, 8));
            const Dword binding = bindings[index];
            set_deadzone_record(records, int(binding >> 24 & 15), int(binding >> 16 & 255),
                                int(binding >> 8 & 255), distance);
        }
        save_current_controller_profile();
    }
    return accepted;
}

Dword open_deadzone_dialog(Dword *bindings, int count) {
    controllers::get_bindings();
    pending::LocalFrontendString type("DeadZoneDialog"), name("Deadzone"),
        file("system\\GDialogs.gui");
    auto *dialog = static_cast<DeadzoneDialog *>(
        pending::create_widget(&file.value, &name.value, &type.value, gui::focused_widget()));
    if (!dialog)
        return 0;
    const Dword result = run_deadzone_dialog(dialog, bindings, count);
    pending::destroy_screen(dialog);
    return result;
}
} // namespace nfs::application::frontend
