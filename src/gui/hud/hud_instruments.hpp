#pragma once
#include "../../frontend/flow/frontend_scene_widgets.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "../widgets/gui_widget_text.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
}
namespace nfs::gui {
kernel::numeric::Extended ticks_to_seconds(int);
}
namespace nfs::gui::widgets {
Dword read_image_resource(void *, const char *, ImageResource *);
Dword write_image_resource(void *, const char *, ImageResource *);
}
namespace nfs::application::frontend::hud {
inline constexpr float metres_per_second_to_mph = 2.2369899749755859375f;
inline constexpr float metres_per_second_to_kph = 3.599999904632568359375f;
inline constexpr float radians_per_second_to_rpm = 9.54929637908935546875f;
inline constexpr float mph_to_kph = 1.60931003093719482421875f;
struct Binding {
    const Method *methods;
    Widget *owner;
};
struct View {
    GeneralWidget base;
    int second_player;
    gui::engine::Point position;
    Dword timer;
};
struct Needle {
    GeneralWidget base;
    Binding binding;
    gui::widgets::ImageResource image;
    SceneVector start, end;
    float start_angle, end_angle, value, target, speed;
    Dword style;
    gui::engine::Rectangle subclip;
};
struct Tachometer {
    Needle base;
    int minimum_rpm, maximum_rpm;
};
struct Speedometer {
    Needle base;
    int minimum_mph, maximum_mph, minimum_kph, maximum_kph;
};
struct Dashboard {
    GeneralWidget base;
    Binding binding;
    gui::String model;
};
static_assert(sizeof(Binding) == 8 && sizeof(View) == 284);
static_assert(sizeof(Needle) == 388 && offsetof(Needle, image) == 272 &&
              offsetof(Needle, value) == 352 && offsetof(Needle, subclip) == 368);
static_assert(sizeof(Tachometer) == 396 && sizeof(Speedometer) == 404 && sizeof(Dashboard) == 292);
extern const Method binding_methods[1];
extern const std::array<Method, 47> view_methods, needle_methods, tachometer_methods,
    speedometer_methods, dashboard_methods;
const char *__cdecl view_class_key();
const char *__cdecl needle_class_key();
const char *__cdecl tachometer_class_key();
const char *__cdecl speedometer_class_key();
const char *__cdecl dashboard_class_key();
Binding *construct_binding(Binding *, Widget *);
void destroy_binding(Binding *);
Dword binding_rule(Binding *, Dword);
void *binding_camera(Binding *);
void *binding_car(Binding *);
View *construct_view(View *, void *);
Needle *construct_needle(Needle *, void *);
void destroy_needle(Needle *);
void *create_view(void *);
void *create_needle(void *);
void *create_tachometer(void *);
void *create_speedometer(void *);
void *create_dashboard(void *);
Dword __fastcall load_view(View *, void *, void *);
Dword __fastcall save_view(View *, void *, void *);
void __fastcall initialize_view(View *, void *);
void __fastcall view_timer(View *, void *, Dword);
void __fastcall tick_view(View *, void *, Dword);
void __fastcall draw_view(View *, void *, void *);
void *view_camera(View *);
void *view_car(View *);
Dword __fastcall load_needle(Needle *, void *, void *);
Dword __fastcall save_needle(Needle *, void *, void *);
void __fastcall tick_needle(Needle *, void *, Dword);
void __fastcall draw_needle(Needle *, void *, void *);
void set_needle_value(Needle *, float, Dword);
void rotate_needle_corners(gui::engine::Point *, gui::engine::Point *, gui::engine::Point *,
                           gui::engine::Point *, float, int, int);
Dword __fastcall load_tachometer(Tachometer *, void *, void *);
Dword __fastcall save_tachometer(Tachometer *, void *, void *);
void __fastcall initialize_tachometer(Tachometer *, void *);
Dword __fastcall tachometer_event(Tachometer *, void *, Dword, Dword, Dword);
void __fastcall tick_tachometer(Tachometer *, void *, Dword);
void update_tachometer(Tachometer *);
Dword __fastcall load_speedometer(Speedometer *, void *, void *);
Dword __fastcall save_speedometer(Speedometer *, void *, void *);
Dword __fastcall speedometer_event(Speedometer *, void *, Dword, Dword, Dword);
void update_speedometer(Speedometer *);
void __fastcall initialize_dashboard(Dashboard *, void *);
Dword __fastcall dashboard_event(Dashboard *, void *, Dword, Dword, Dword);
void __fastcall tick_dashboard(Dashboard *, void *, Dword);
void reload_dashboard(Dashboard *);
}
namespace nfs::game::world::camera {
kernel::Dword top_mode(void *);
kernel::Dword hud_visible(void *);
}
namespace nfs::application::pending {
Byte participant_blocked(void *, Dword);
}
