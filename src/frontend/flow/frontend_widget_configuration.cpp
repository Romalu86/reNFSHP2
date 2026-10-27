#include "nfs/core/application/application.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../gui/widgets/gui_widget_controls.hpp"
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
void *find_child(void *, const char *);
void set_widget_visible(void *, Dword);
void bring_widget_to_front(void *);
void refresh_camera_layout(void *);
const char *active_hud_name{};
const char *netgame_frontend_class_key();
namespace {
struct ScreenString {
    gui::String value;
    explicit ScreenString(const char *text) {
        gui::string_construct_ascii(&value, text);
    }
    ~ScreenString() {
        gui::string_destruct(&value);
    }
};
void *create_interface_child(const char *type, const char *name, void *parent) {
    ScreenString class_name(type), widget_name(name), resource("System\\GInterface.gui");
    return create_widget(&resource.value, &widget_name.value, &class_name.value, parent);
}
} // namespace
// The ECX frontend receiver and first stack word are independent.
void configure_race_widgets(void *, Dword visible, void *screen) {
    if (!screen)
        return;
    void *countdown = find_child(screen, "<COUNTDOWN>");
    if (!countdown)
        countdown = create_interface_child("HP2CountDown", "<COUNTDOWN>", screen);
    if (!countdown)
        return;
    gui::widgets::family::move_to_first_draw_position(
        static_cast<gui::widgets::Widget *>(countdown));
    set_widget_visible(countdown, visible);
}

void prepare_simulation_widgets(void *, void *screen) {
    if (!screen || !network::session_active())
        return;
    void *netgame = find_child(screen, "<NET>");
    if (!netgame)
        netgame = create_interface_child("NetgameFE", "<NET>", screen);
    else
        set_widget_visible(netgame, 1);
    void *typed =
        netgame && invoke<Dword>(netgame, 2, netgame_frontend_class_key) ? netgame : nullptr;
    at<void *>(system(104), 92) = typed;
}

void configure_player_widgets(void *, Dword visible, void *screen) {
    if (!graphics::device || !screen)
        return;
    const char *name;
    if (!game::runtime::physics::split_screen()) {
        name = "<HUD>";
    } else {
        name = network::packets::rule_value(system(72), 38) ? "<HUD2PH>" : "<HUD2PV>";
        for (void *camera : game::tick_services)
            if (camera)
                refresh_camera_layout(camera);
    }
    void *hud = find_child(screen, name);
    if (visible) {
        if (active_hud_name && active_hud_name != name)
            if (void *previous = find_child(screen, active_hud_name))
                destroy_screen(previous);
        active_hud_name = name;
        if (!hud)
            hud = create_interface_child("GWidget", name, screen);
        else
            set_widget_visible(hud, visible);
        gui::widgets::set_enabled(static_cast<gui::widgets::Widget *>(hud), 0);
        bring_widget_to_front(hud);
    } else if (hud) {
        destroy_screen(hud);
    }
}
// Class-key leaf.
const char *netgame_frontend_class_key() {
    return "NetgameFE";
}
} // namespace nfs::application::pending
namespace nfs::services::pending {

void load_frontend_screens(void *frontend, nfs::kernel::Dword view) {
    application::pending::create_frontend_view(frontend, static_cast<std::uint8_t>(view));
}
} // namespace nfs::services::pending
