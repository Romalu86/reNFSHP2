#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_race_loading.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/session/network_rules.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

static Dword __cdecl loading_controller_setting(void *sender, Dword, Dword, Dword) {
    pending::LocalFrontendString empty;
    gui::string_reference(&empty.value, 758);
    invoke<void>(sender, 7, &empty.value);
    pending::set_widget_visible(sender, 1);
    struct Caption { int input; Dword text; };
    Caption axes[] = {{1,1062},{0,1063},{2,344},{3,344},{4,1274},{5,1274},
                      {6,570},{7,571},{8,337},{9,336},{10,575},{14,572},
                      {11,576},{15,1309},{16,1310},{17,1311},{26,0}};
    Caption buttons[] = {{1,341},{0,341},{2,344},{3,344},{4,331},{5,331},
                         {6,570},{7,571},{8,337},{9,336},{10,575},{14,572},
                         {11,576},{15,1309},{16,1310},{17,1311},{26,0}};
    const bool quota = network::packets::rule_value(system(72), 2) == 10;
    for (Caption *table : {axes, buttons})
        for (auto *caption = table; caption->text; ++caption) {
            if (quota && caption->input == 14)
                caption->text = 1275;
            else if (!quota && caption->input >= 15 && caption->input <= 17)
                caption->text = 0;
        }
    struct Binding { const char *name; bool axis; int input; Dword text; };
    const Binding bindings[] = {{"<AXIS_0_X>",false,2,758},
                                {"<AXIS_1_X>",false,6,758},
                                {"<RIGHT_THUMB_UP>",false,4,758},
                                {"<RIGHT_THUMB_DOWN>",false,5,758}};
    const auto *name = reinterpret_cast<const gui::String *>(game::runtime::bytes(sender) + 72);
    int wanted = -1;
    bool axis = false;
    for (const auto &binding : bindings)
        if (!gui::string_compare_inline_ascii(name, binding.name)) {
            wanted = binding.input;
            axis = binding.axis;
        }
    if (wanted < 0)
        return 0;
    auto *controls = game::runtime::bytes(system(72)) + 2544;
    for (int input = 0; input < 26; ++input) {
        const Dword binding = at<Dword>(controls, input * 4);
        if (axis ? (binding & 255) != 2 || int((binding >> 8) & 1048575) != wanted
                 : (binding & 255) != 1 || int((binding >> 24) & 15) != wanted)
            continue;
        Caption *table = axis ? axes : buttons;
        if (!table->text)
            continue;
        for (auto *caption = table;; ++caption) {
            if (caption->input == input) {
                pending::LocalFrontendString text;
                gui::string_reference(&text.value, caption->text);
                invoke<void>(sender, 7, &text.value);
                return 0;
            }
            if (!(caption + 1)->text)
                break;
        }
    }
    return 0;
}

static Dword __cdecl loading_ea_tip(void *sender, Dword, Dword, Dword) {
    {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, network::rules::random() % 20 + 1514);
        invoke<void>(sender, 7, &text.value);
    }
    invoke<void>(sender, 6, Dword(64));
    return 0;
}
static CallbackRecord loading_callbacks[2];

static void unregister_loading_callbacks() {
    for (unsigned i = 2; i; --i)
        unregister_callback(loading_callbacks + i - 1);
}

static void register_loading_callbacks() {
    register_callback(loading_callbacks, base_class_name, "GEVENT_BEGIN", 18,
                      "Loading_EATip", loading_ea_tip, 0);
    register_callback(loading_callbacks + 1, base_class_name, "GEVENT_BEGIN", 18,
                      "Loading_ControllerSetting", loading_controller_setting, 0);
    std::atexit(unregister_loading_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_loading_callbacks, ".CRT$XCU_NFS_006900C8",
                       register_loading_callbacks);
} // namespace
} // namespace nfs::application::frontend
