#include "gui_script.hpp"
#include "../widgets/gui_widget_sequence.hpp"
namespace nfs::gui::widgets::script::commands {
using application::pending::LocalFrontendString;
namespace {
struct TemporaryImage {
    ImageResource value;
    TemporaryImage() {
        construct_image_resource(&value);
    }
    ~TemporaryImage() {
        destroy_image_resource(&value);
    }
};
struct TemporarySequence {
    family::ImageSequence value;
    TemporarySequence() {
        family::construct_image_sequence(&value);
    }
    ~TemporarySequence() {
        family::destroy_image_sequence(&value);
    }
};
} // namespace

Dword __cdecl set_image(Script *script, Command *command, int) {
    LocalFrontendString name(argument(command, 0, script));
    LocalFrontendString variant(argument(command, 1, script));
    if (Widget *widget = target(script))
        if (string_data(&name.value) && active_engine()) {
            if (string_data(&variant.value)) {
                TemporaryImage image;
                {
                    LocalFrontendString prefix, closed, qualified;
                    prepend_ascii_value(&prefix.value, "[", &name.value);
                    append_ascii_value(&closed.value, &prefix.value, "] ");
                    concatenate(&qualified.value, &closed.value, &variant.value);
                    load_image_resource(&image.value, &qualified.value);
                }
                events::dispatch(widget, 52, reinterpret_cast<Dword>(&image.value), 0);
            } else {
                TemporarySequence sequence;
                family::load_image_sequence(&sequence.value, &name.value, 0);
                events::dispatch(widget, 50, reinterpret_cast<Dword>(&sequence.value), 0);
            }
        }
    return 1;
}
} // namespace nfs::gui::widgets::script::commands
