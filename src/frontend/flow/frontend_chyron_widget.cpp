#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_application_widgets.hpp"
#include "frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_settings.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdlib>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using kernel::numeric::Extended;
struct Chyron {
    GeneralWidget base;
    Widget *title, *artist, *brand, *album;
    int display_seconds, fade_rate;
    Dword elapsed;
    gui::String title_text, artist_text, album_text;
    Dword timer_id;
};
static_assert(sizeof(Chyron) == 356 && offsetof(Chyron, title_text) == 292);
extern const std::array<Method, 47> chyron_methods;

static const char *__cdecl chyron_class_key() {
    return "ChyronCtl";
}

static const char *__fastcall chyron_name(Widget *, void *) {
    return "ChyronCtl";
}

static Dword __fastcall chyron_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(chyron_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall chyron_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "ChyronCtl", 10) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static Chyron *construct_chyron(Chyron *widget, void *parent) {
    construct_widget(widget, parent);
    widget->title = widget->artist = widget->brand = widget->album = nullptr;
    widget->display_seconds = widget->fade_rate = 0;
    widget->elapsed = 0;
    try {
        gui::string_construct(&widget->title_text);
        try {
            gui::string_construct(&widget->artist_text);
            try {
                gui::string_construct(&widget->album_text);
                try {
                    widget->timer_id = 0;
                    widget->base.base.methods = chyron_methods.data();
                    widget->timer_id = gui::create_widget_timer(widget->base.base.id, 1.0f);
                } catch (...) {
                    gui::string_destruct(&widget->album_text);
                    throw;
                }
            } catch (...) {
                gui::string_destruct(&widget->artist_text);
                throw;
            }
        } catch (...) {
            gui::string_destruct(&widget->title_text);
            throw;
        }
    } catch (...) {
        destroy_general(&widget->base);
        throw;
    }
    return widget;
}

static void destroy_chyron(Chyron *widget) {
    widget->base.base.methods = chyron_methods.data();
    gui::remove_widget_timer(widget->timer_id);
    gui::string_destruct(&widget->album_text);
    gui::string_destruct(&widget->artist_text);
    gui::string_destruct(&widget->title_text);
    destroy_general(&widget->base);
}

static Chyron *__fastcall delete_chyron(Chyron *widget, void *, Dword flags) {
    destroy_chyron(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static void __fastcall initialize_chyron(Chyron *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    auto *root = &widget->base.base;
    application::pending::set_widget_visible(root, 0);
    const char *names[] = {"<TITLE>", "<ARTIST>", "<BRAND>", "<ALBUM>"};
    Widget **fields[] = {&widget->title, &widget->artist, &widget->brand, &widget->album};
    for (unsigned index = 0; index != 4; ++index) {
        auto *child = static_cast<Widget *>(application::pending::find_child(root, names[index]));
        if (!child || !invoke<Dword>(child, 2, family::text_class_key))
            child = nullptr;
        *fields[index] = child;
        if (child)
            application::pending::set_widget_visible(child, 0);
    }
}

static Dword __fastcall load_chyron(Chyron *widget, void *, void *configuration) {
    gui::engine::pending::read_integer(configuration, "ChyronCtl.m_DisplayTime",
                                       &widget->display_seconds, 0);
    gui::engine::pending::read_integer(configuration, "ChyronCtl.m_FadeRate", &widget->fade_rate,
                                       0);
    return gui::widgets::pending::load_general(&widget->base, nullptr, configuration);
}

static Dword __fastcall save_chyron(Chyron *widget, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "ChyronCtl.m_DisplayTime",
                                    widget->display_seconds, 0);
    gui::engine::pending::write_int(configuration, "ChyronCtl.m_FadeRate", widget->fade_rate, 0);
    return gui::widgets::pending::save_general(&widget->base, nullptr, configuration);
}

static void show_music_metadata(Chyron *widget, const gui::String *title, const gui::String *artist,
                                const gui::String *album, const gui::String *brand) {
    std::uint8_t visible = 0;
    if (widget->title) {
        application::pending::LocalFrontendString quoted("\"");
        gui::string_append(&quoted.value, title);
        gui::string_append_ascii(&quoted.value, "\"");
        invoke(widget->title, 7, &quoted.value);
        application::pending::set_widget_visible(widget->title, 1);
        visible = 1;
    }
    Widget *fields[] = {widget->artist, widget->brand, widget->album};
    const gui::String *strings[] = {artist, brand, album};
    for (unsigned index = 0; index != 3; ++index)
        if (fields[index]) {
            invoke(fields[index], 7, strings[index]);
            application::pending::set_widget_visible(fields[index], 1);
            visible = 1;
        }
    if (!game::audio::state::configuration<int>(audio_layout::music_volume) ||
        game::audio::state::configuration<int>(audio_layout::music_muted))
        visible = 0;
    application::pending::set_widget_visible(widget, visible);
    widget->elapsed = 0;
    widget->display_seconds = 4;
}

static void __fastcall update_chyron(Chyron *widget, void *, Dword elapsed) {
    if (!widget->title || (widget->title->flags & 2))
        return;
    family::move_to_first_draw_position(&widget->base.base);
    const auto format = kernel::numeric::current_format();
    const float seconds = float(Extended::integer(widget->display_seconds, format));
    widget->elapsed += elapsed;
    const int timeout = gui::engine::seconds_to_ticks(seconds);
    if (int(widget->elapsed) <= timeout || (widget->title->flags & 2))
        return;
    Widget *fields[] = {widget->title, widget->album, widget->artist, widget->brand};
    for (auto *child : fields)
        if (child)
            application::pending::set_widget_visible(child, 0);
    application::pending::set_widget_visible(widget, 0);
}

static Dword __fastcall chyron_event(Chyron *widget, void *, Dword event, Dword first, Dword second) {
    auto &library = game::audio::state::library();
    if (event == 22 && first == widget->timer_id && std::uint8_t(library.notify)) {
        reinterpret_cast<std::uint8_t *>(&library.notify)[0] = 0;
        gui::String brand;
        gui::string_construct_reference(&brand, 1538);
        try {
            application::pending::LocalFrontendString album(library.album), artist(library.artist),
                title(library.title);
            show_music_metadata(widget, &title.value, &artist.value, &album.value, &brand);
        } catch (...) {
            gui::string_destruct(&brand);
            throw;
        }
        gui::string_destruct(&brand);
    }
    return gui::widgets::pending::general_event(&widget->base, nullptr, event, first, second);
}

const std::array<Method, 47> chyron_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_chyron), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::chyron_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::chyron_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::chyron_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_chyron), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_chyron), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::chyron_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_chyron), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_chyron), // 24.
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

static void *create_chyron(void *parent) {
    auto *storage = static_cast<Chyron *>(gui::allocate_flags(sizeof(Chyron), 0, "ChyronCtl"));
    if (!storage)
        return nullptr;
    try {
        return construct_chyron(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord chyron_class;

static void unregister_chyron() {
    unregister_class(&chyron_class);
}

static void register_chyron() {
    register_class(&chyron_class, "ChyronCtl", create_chyron, "GWidget");
    std::atexit(unregister_chyron);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_chyron, ".CRT$XCU_NFS_00690058",
                       register_chyron);
} // namespace
} // namespace nfs::application::frontend
