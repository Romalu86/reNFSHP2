#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_instruments.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static Needle *__fastcall delete_needle(Needle *, void *, Dword);

static Needle *__fastcall delete_needle_binding(Binding *binding, void *, Dword flags) {
    return delete_needle(reinterpret_cast<Needle *>(reinterpret_cast<Byte *>(binding) - 264), nullptr, flags);
}
static const Method needle_binding_methods[1] = {reinterpret_cast<Method>(delete_needle_binding)};

Needle *construct_needle(Needle *needle, void *parent) {
    construct_widget(needle, parent);
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_general(&needle->base); });
    construct_binding(&needle->binding, &needle->base.base);
    kernel::lifecycle::Cleanup binding_cleanup([=] { destroy_binding(&needle->binding); });
    construct_image_resource(&needle->image);
    kernel::lifecycle::Cleanup image_cleanup([=] { destroy_image_resource(&needle->image); });
    needle->start = {game::runtime::values::vector_methods, 0, 0, 0};
    kernel::lifecycle::Cleanup start_cleanup([=] { game::runtime::values::destroy_vector(&needle->start); });
    needle->end = {game::runtime::values::vector_methods, 0, 0, 0};
    kernel::lifecycle::Cleanup end_cleanup([=] { game::runtime::values::destroy_vector(&needle->end); });
    needle->start_angle = needle->end_angle = needle->value = needle->target = 0;
    needle->speed = 5;
    needle->style = 0;
    gui::engine::construct_rectangle(&needle->subclip);
    kernel::lifecycle::Cleanup clip_cleanup([=] { gui::engine::destroy_rectangle(&needle->subclip); });
    needle->base.base.methods = needle_methods.data();
    needle->binding.methods = needle_binding_methods;
    needle->start.x = 1;
    needle->end.x = 1;
    needle->base.base.flags = (needle->base.base.flags & ~Dword(4)) | 1;
    clip_cleanup.dismiss();
    end_cleanup.dismiss();
    start_cleanup.dismiss();
    image_cleanup.dismiss();
    binding_cleanup.dismiss();
    base_cleanup.dismiss();
    return needle;
}

void destroy_needle(Needle *needle) {
    needle->base.base.methods = needle_methods.data();
    needle->binding.methods = needle_binding_methods;
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_general(&needle->base); });
    kernel::lifecycle::Cleanup binding_cleanup([=] { destroy_binding(&needle->binding); });
    kernel::lifecycle::Cleanup image_cleanup([=] { destroy_image_resource(&needle->image); });
    kernel::lifecycle::Cleanup start_cleanup([=] { game::runtime::values::destroy_vector(&needle->start); });
    kernel::lifecycle::Cleanup end_cleanup([=] { game::runtime::values::destroy_vector(&needle->end); });
    gui::engine::destroy_rectangle(&needle->subclip);
}

static Needle *__fastcall delete_needle(Needle *needle, void *, Dword flags) {
    destroy_needle(needle);
    if (flags & 1) gui::release(needle);
    return needle;
}

const char *__cdecl needle_class_key() { return "MNeedle"; }

static const char *__fastcall needle_name(Needle *, void *) { return "MNeedle"; }

static Dword __fastcall needle_is_type(Needle *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(needle_class_key) ||
        type == reinterpret_cast<const void *>(general_class_name) || type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall needle_is_name(Needle *, void *, const char *name) {
    return !std::strncmp(name, "MNeedle", 8) || !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> needle_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_needle), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::needle_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::needle_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::needle_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_needle), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_needle), // 5.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::draw_needle), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tick_needle), // 24.
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

void *create_needle(void *parent) {
    auto *needle = static_cast<Needle *>(gui::allocate_flags(sizeof(Needle), 0, "MNeedle"));
    if (!needle) return nullptr;
    try { return construct_needle(needle, parent); }
    catch (...) { gui::release_flags(needle, 0, "MNeedle"); throw; }
}
}
