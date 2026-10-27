#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstring>
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_control_remap(ControlRemapDialog *dialog, void *) {
    family::msg_box_initialize(&dialog->base, nullptr);
    for (int device = 0; device < 16; ++device)
        if (void *state = input::device_get(device, 6))
            if (at<std::uint8_t>(state, 0) == 4)
                std::memcpy(dialog->snapshots[device], state, 260);
    auto *text = static_cast<Widget *>(pending::find_child(dialog, "<TEXT>"));
    if (text && invoke<Dword>(text, 2, family::text_class_key))
        invoke(text, 7, &lifecycle::construction::unresolved::shared_string);
    dialog->input = system(100);
}

Dword __fastcall accept_control_remap(ControlRemapDialog *dialog, void *, Dword first, Dword second) {
    if (std::uint8_t(dialog->binding)) {
        family::finish_modal(&dialog->base, 2, 0);
        return 1;
    }
    return style_result_34(&dialog->base, nullptr, first, second);
}
// By-value String is consumed and destroyed.
void set_control_remap_prompt(ControlRemapDialog *dialog, gui::String text) {
    struct Owner {
        gui::String *text;
        ~Owner() {
            gui::string_destruct(text);
        }
    } owner{&text};
    if (auto *child = static_cast<Widget *>(pending::find_child(dialog, "<TEXT>")))
        invoke<Dword>(child, 2, family::text_class_key);
}
// Only AL is consumed by callers.
static std::uint8_t remappable_scan_code(Dword scan) {
    switch (scan) {
    case 14:
    case 41:
    case 70:
    case 183:
    case 197:
    case 219:
    case 220:
    case 221:
        return 0;
    default:
        return 1;
    }
}
// AL is the direction used by the callers.
static std::uint8_t cardinal_pov(int value) {
    if (value == -1)
        return 0;
    if (value == 0)
        return 1;
    if (value == 18000)
        return 2;
    if (value == 27000)
        return 3;
    return value == 9000 ? 4 : 0;
}

void __fastcall update_control_remap(ControlRemapDialog *dialog, void *, Dword elapsed) {
    dialog->elapsed += elapsed;
    if (int(dialog->elapsed) <= gui::engine::seconds_to_ticks(0.1f))
        return;
    if (std::uint8_t(dialog->binding) && dialog->released) {
        family::finish_modal(&dialog->base, 2, 0);
        return;
    }
    dialog->binding = dialog->value = 0;
    game::poll_input();
    void *bindings = controllers::get_bindings();
    for (int device = 0; device < 16; ++device) {
        auto *state = static_cast<Dword *>(input::device_get(device, 6));
        if (!state)
            continue;
        const auto kind = std::uint8_t(*state);
        if (kind == 3) {
            const auto *keys = reinterpret_cast<const std::uint8_t *>(state) + 4;
            for (Dword scan = 0; scan < 223; ++scan)
                if (keys[scan] && remappable_scan_code(scan)) {
                    dialog->binding = 4 | (scan << 8);
                    return;
                }
        } else if (kind == 4) {
            void *about = input::device_get(device, 2);
            if (!about)
                continue;
            const Dword button_mask = at<Dword>(about, 12), axis_mask = at<Dword>(about, 8),
                       pov_mask = at<Dword>(about, 16);
            const auto *buttons = reinterpret_cast<const std::uint8_t *>(state) + 52;
            for (Dword button = 0; button < 32; ++button)
                if ((button_mask & (Dword(1) << button)) && buttons[button]) {
                    const Dword binding = 2 | (button << 8) | (Dword(device) << 28);
                    if (resolve_menu_control(bindings, &binding, 0, false)) {
                        dialog->binding = binding;
                        return;
                    }
                }
            for (Dword axis = 0; axis < 8; ++axis) {
                if (!(axis_mask & (Dword(1) << axis)))
                    continue;
                const int initial = int(dialog->snapshots[device][axis + 1]) >> 8,
                          current = int(state[axis + 1]) >> 8;
                const Dword difference = Dword(current) - Dword(initial);
                const Dword sign = Dword(int(difference) >> 31);
                const int distance = int((difference ^ sign) - sign);
                const int threshold = initial == -1 ? 1 : 10;
                if (distance < threshold)
                    continue;
                const Dword binding = 65281 | ((axis & 15) << 24) | (Dword(device) << 28);
                void *control = resolve_menu_control(bindings, &binding, Dword(current), true);
                if (!control)
                    continue;
                void *definition = invoke<void *>(control, 3);
                if (!definition)
                    continue;
                dialog->binding = 1 | ((at<Dword>(definition, 20) & 255) << 8) |
                                  ((at<Dword>(definition, 16) & 255) << 16) | ((axis & 15) << 24) |
                                  (Dword(device) << 28);
                dialog->value = Dword(current);
                return;
            }
            for (Dword pov = 0; pov < 4; ++pov) {
                if (!(pov_mask & (Dword(1) << pov)))
                    continue;
                const Dword direction = state[pov + 9];
                if (!cardinal_pov(int(direction)))
                    continue;
                const Dword binding =
                    3 | ((direction & 65535) << 8) | ((pov & 15) << 24) | (Dword(device) << 28);
                if (resolve_menu_control(bindings, &binding, direction, true)) {
                    dialog->binding = binding;
                    dialog->value = direction;
                    return;
                }
            }
        }
    }
    if (!std::uint8_t(dialog->binding))
        dialog->released = 1;
}
} // namespace nfs::application::frontend
