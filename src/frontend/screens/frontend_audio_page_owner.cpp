#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_pages.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
extern const std::array<Method, 47> audio_page_methods;

const char *__cdecl audio_page_class_key() {
    return "AudioScreen";
}

static const char *__fastcall audio_page_name(Widget *, void *) {
    return "AudioScreen";
}

static Dword __fastcall audio_page_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(audio_page_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall audio_page_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "AudioScreen", 12) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_audio_page(AudioPage *page) {
    page->base.base.methods = audio_page_methods.data();
    destroy_general(&page->base);
}

static AudioPage *__fastcall delete_audio_page(AudioPage *page, void *, Dword flags) {
    destroy_audio_page(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 47> audio_page_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_audio_page), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_page_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_page_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_page_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_audio_page), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_audio_page), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_audio_page), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_page_key), // 32.
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

static void *create_audio_page(void *parent) {
    auto *page = static_cast<AudioPage *>(gui::allocate_flags(sizeof(AudioPage), 0, "AudioScreen"));
    if (!page)
        return nullptr;
    try {
        construct_widget(page, parent);
        page->menu_music = page->race_music = page->engine = page->voice = page->effects = nullptr;
        page->mode_widget = 0;
        for (auto &level : page->initial_levels)
            level = 0;
        page->mode = page->playlist = 0;
        page->suppress_release_fade = 0;
        page->base.base.methods = audio_page_methods.data();
        return page;
    } catch (...) {
        gui::release(page);
        throw;
    }
}
static ClassRecord audio_page_class;

static void unregister_audio_page() {
    unregister_class(&audio_page_class);
}

static void register_audio_page() {
    register_class(&audio_page_class, "AudioScreen", create_audio_page, "GWidget");
    std::atexit(unregister_audio_page);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_audio_page, ".CRT$XCU_NFS_00690048",
                       register_audio_page);
} // namespace
} // namespace nfs::application::frontend
