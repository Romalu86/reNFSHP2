#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_particle_widget.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui {
void update_widget_timer(Dword, float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
static float default_velocity[2];

static const char *__cdecl particle_class_key() {
    return "FEParticle";
}

static const char *__fastcall particle_name(Widget *, void *) {
    return "FEParticle";
}

static Dword __fastcall particle_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(particle_class_key) ||
           type == reinterpret_cast<const void *>(family::image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall particle_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "FEParticle", 11) || !std::strncmp(name, "GImage", 7) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static ParticleWidget *construct_particle_widget(ParticleWidget *widget, void *parent) {
    family::construct_image_widget(widget, parent);
    widget->period = 0.0333333350718021392822265625f;
    widget->update_timer = widget->spawn_timer = widget->serial = 0;
    widget->particles = nullptr;
    try {
        construct_image_resource(&widget->image);
        try {
            widget->maximum_particles = 0;
            widget->start_colour = widget->middle_colour = widget->end_colour = 0;
            widget->velocity[0] = widget->drag[0] = widget->acceleration[0] = default_velocity[0];
            widget->spawn_rate = widget->life = 0;
            widget->middle_life = 0.5f;
            widget->fade_in = widget->fade_out = 1;
            widget->angle = widget->angle_randomness = widget->angular_velocity = 0;
            widget->angular_spread = widget->angular_drag = 0;
            widget->velocity[1] = widget->drag[1] = widget->acceleration[1] = default_velocity[1];
            widget->spread = widget->scatter_x = widget->scatter_y = 0;
            widget->scale = 1;
            widget->growth = 0;
            gui::engine::construct_point(&widget->centre);
            try {
                widget->sort_direction = 1;
                widget->blend = widget->style = 0;
                widget->base.base.methods = particle_widget_methods.data();
                const Dword id = widget->base.base.id;
                widget->update_timer = gui::create_widget_timer(id, widget->period);
                widget->spawn_timer = gui::create_widget_timer(id, 0);
            } catch (...) {
                gui::engine::destroy_point(&widget->centre);
                throw;
            }
        } catch (...) {
            destroy_image_resource(&widget->image);
            throw;
        }
    } catch (...) {
        family::destroy_image_widget(&widget->base.base);
        throw;
    }
    return widget;
}

static void destroy_particle_widget(ParticleWidget *widget) {
    widget->base.base.methods = particle_widget_methods.data();
    if (widget->particles) {
        kernel::memory_free(widget->particles);
        widget->particles = nullptr;
    }
    gui::remove_widget_timer(widget->update_timer);
    gui::remove_widget_timer(widget->spawn_timer);
    gui::engine::destroy_point(&widget->centre);
    destroy_image_resource(&widget->image);
    family::destroy_image_widget(&widget->base.base);
}

static ParticleWidget *__fastcall delete_particle_widget(ParticleWidget *widget, void *,
                                                         Dword flags) {
    destroy_particle_widget(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

void allocate_menu_particles(ParticleWidget *widget) {
    if (widget->particles) {
        kernel::memory_free(widget->particles);
        widget->particles = nullptr;
    }
    if (widget->image.image && widget->spawn_rate != 0) {
        const Dword size = Dword(widget->maximum_particles) * sizeof(MenuParticle);
        widget->particles = static_cast<MenuParticle *>(
            kernel::memory_allocate("FEParticle", int(size), lifecycle::ui_pool));
        if (widget->particles)
            kernel::memory_zero(widget->particles, size);
        gui::update_widget_timer(widget->spawn_timer, menu_particle_period(widget->spawn_rate));
    }
}

static void __fastcall initialize_particle_widget(ParticleWidget *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    if (!(widget->style & 1))
        warm_up_menu_particles(widget);
    if (widget->style & 6)
        application::pending::set_widget_visible(widget, 0);
}

static void __fastcall particle_timer(ParticleWidget *widget, void *, Dword timer) {
    if (!family::visible_in_ancestors(&widget->base.base))
        return;
    if (timer == widget->update_timer)
        advance_menu_particles(widget, widget->period);
    if (timer == widget->spawn_timer)
        spawn_menu_particle(widget);
}

static void __fastcall prepare_particle_widget(ParticleWidget *widget, void *,
                                               gui::engine::Engine *render) {
    if (widget->style & 6) {
        Dword visible = 0;
        auto *focused =
            static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
        if (focused) {
            if ((widget->style & 2) && has_ancestor(&widget->base.base, focused))
                visible = 1;
            if ((widget->style & 4) && family::sibling_by_id(&widget->base.base, focused->id))
                visible = 1;
        }
        application::pending::set_widget_visible(widget, visible);
    }
    prepare_draw(&widget->base.base, nullptr, render);
}

const std::array<Method, 47> particle_widget_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_particle_widget), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::particle_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::particle_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::particle_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_particle_widget), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_particle_widget), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::application::frontend::prepare_particle_widget), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_push_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_particle_widget), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_particle_widget), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::particle_timer), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

static void *create_particle_widget(void *parent) {
    auto *storage =
        static_cast<ParticleWidget *>(gui::allocate_flags(sizeof(ParticleWidget), 0, "FEParticle"));
    if (!storage)
        return nullptr;
    try {
        return construct_particle_widget(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord particle_class;

static void unregister_particle_widget() {
    unregister_class(&particle_class);
}

static void register_particle_widget() {
    register_class(&particle_class, "FEParticle", create_particle_widget, "GWidget");
    std::atexit(unregister_particle_widget);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_particle_widget, ".CRT$XCU_NFS_00690084",
                       register_particle_widget);
} // namespace
} // namespace nfs::application::frontend
