#include "hud_instruments.hpp"
#include "../widgets/gui_widget_menu.hpp"
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
using game::runtime::bytes;

static void clear_dashboard_children(Widget *widget) {
    gui::container_clear(&widget->draw_order);
    gui::container_clear(&widget->tab_order);
    gui::container_clear(&widget->children);
}

void reload_dashboard(Dashboard *dashboard) {
    if (void *car = binding_car(&dashboard->binding)) {
        const char *model = reinterpret_cast<const char *>(bytes(car) + 1532);
        if (!gui::string_compare_ascii(&dashboard->model, model)) return;
        clear_dashboard_children(&dashboard->base.base);
        pending::LocalFrontendString resource("pc\\pc.hud.dash.");
        gui::string_append_ascii(&resource.value, model);
        family::reload_widget_children(&dashboard->base.base, gui::string_ascii(&resource.value));
        gui::string_assign(&dashboard->model, model);
    }
}

void __fastcall initialize_dashboard(Dashboard *dashboard, void *) {
    gui::widgets::pending::request_general_layout(&dashboard->base, nullptr);
    reload_dashboard(dashboard);
}

Dword __fastcall dashboard_event(Dashboard *dashboard, void *, Dword event, Dword first, Dword second) {
    if (event == 104) reload_dashboard(dashboard);
    return gui::widgets::pending::general_event(&dashboard->base, nullptr, event, first, second);
}

void __fastcall tick_dashboard(Dashboard *dashboard, void *, Dword) {
    void *camera = binding_camera(&dashboard->binding);
    pending::set_widget_visible(dashboard, camera && game::world::camera::top_mode(camera) == 3);
}
}
