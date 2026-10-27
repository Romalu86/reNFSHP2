#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_scene_widgets.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl actor_preview_class_key() {
    return "ActorWidget";
}

static const char *__fastcall actor_preview_name(Widget *, void *) {
    return "ActorWidget";
}

static Dword __fastcall actor_preview_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(actor_preview_class_key) ||
           type == reinterpret_cast<const void *>(scene_widget_class_key) ||
           type == reinterpret_cast<const void *>(family::image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall actor_preview_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "ActorWidget", 12) || !std::strncmp(name, "EAGLWidget", 11) ||
           !std::strncmp(name, "GImage", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static ActorPreview *__fastcall delete_actor_preview(ActorPreview *page, void *, Dword flags) {
    destroy_actor_preview(page);
    if (flags & 1)
        gui::release(page);
    return page;
}
const std::array<Method, 47> actor_preview_methods = {
    reinterpret_cast<Method>(delete_actor_preview),
    reinterpret_cast<Method>(actor_preview_name),
    reinterpret_cast<Method>(actor_preview_is_type),
    reinterpret_cast<Method>(actor_preview_is_name),
    reinterpret_cast<Method>(load_actor_preview),
    reinterpret_cast<Method>(save_actor_preview),
    reinterpret_cast<Method>(gui::widgets::pending::layout),
    reinterpret_cast<Method>(set_text),
    reinterpret_cast<Method>(text),
    reinterpret_cast<Method>(gui::widgets::pending::hit_test),
    reinterpret_cast<Method>(gui::widgets::pending::navigate),
    reinterpret_cast<Method>(gui::widgets::pending::enter_modal),
    reinterpret_cast<Method>(gui::widgets::pending::leave_modal),
    reinterpret_cast<Method>(gui::widgets::pending::general_event),
    reinterpret_cast<Method>(reserved_four_arguments),
    reinterpret_cast<Method>(reserved_two_arguments),
    reinterpret_cast<Method>(cursor_scene_widget),
    reinterpret_cast<Method>(prepare_draw),
    reinterpret_cast<Method>(family::static_image_push_bounds),
    reinterpret_cast<Method>(draw_actor_preview),
    reinterpret_cast<Method>(pop_draw_bounds),
    reinterpret_cast<Method>(initialize_actor_preview),
    reinterpret_cast<Method>(reserved_no_arguments),
    reinterpret_cast<Method>(tick_actor_preview),
    reinterpret_cast<Method>(reserved_style_argument),
    reinterpret_cast<Method>(release_scene_widget),
    reinterpret_cast<Method>(reserved_layout_argument),
    reinterpret_cast<Method>(leave_scene_widget),
    reinterpret_cast<Method>(notify_parent_leave),
    reinterpret_cast<Method>(reserved_parent_argument),
    reinterpret_cast<Method>(reserved_child_argument),
    reinterpret_cast<Method>(set_modal_state),
    reinterpret_cast<Method>(gui::widgets::pending::general_key),
    reinterpret_cast<Method>(reserved_result),
    reinterpret_cast<Method>(style_result_34),
    reinterpret_cast<Method>(style_result_35),
    reinterpret_cast<Method>(gui::widgets::pending::general_button),
    reinterpret_cast<Method>(style_result_37),
    reinterpret_cast<Method>(style_result_38),
    reinterpret_cast<Method>(drag_scene_widget),
    reinterpret_cast<Method>(press_scene_widget),
    reinterpret_cast<Method>(release_scene_pointer),
    reinterpret_cast<Method>(accept_general),
    reinterpret_cast<Method>(cancel_general),
    reinterpret_cast<Method>(notify_scroll),
    reinterpret_cast<Method>(process_general_command),
    reinterpret_cast<Method>(process_general_notification)};

void *create_actor_preview(void *parent) {
    auto *page =
        static_cast<ActorPreview *>(gui::allocate_flags(sizeof(ActorPreview), 0, "ActorWidget"));
    if (!page)
        return nullptr;
    try {
        construct_actor_preview(page, parent);
        reinterpret_cast<Widget *>(page)->methods = actor_preview_methods.data();
        return page;
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord actor_preview_class;

static void unregister_actor_preview() {
    unregister_class(&actor_preview_class);
}

static void register_actor_preview() {
    register_class(&actor_preview_class, "ActorWidget", create_actor_preview, "EAGLWidget");
    std::atexit(unregister_actor_preview);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_actor_preview, ".CRT$XCU_NFS_00690040",
                       register_actor_preview);
} // namespace
} // namespace nfs::application::frontend
