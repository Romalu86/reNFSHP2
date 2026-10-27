#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_application_widgets.hpp"
#include "../../gui/widgets/gui_widget_settings.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/timing/timer.hpp"
#include "../../gui/lifecycle/gui_timer_x87.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui {
List *construct_default_list(List *);
} // namespace nfs::gui
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
namespace pending {
Rectangle *intersect_rectangle(Rectangle *, const Rectangle *, const Rectangle *);
}
} // namespace nfs::gui::engine
namespace nfs::gui::widgets {
void __fastcall push_draw_bounds(Widget *, void *, engine::Engine *);
void __fastcall pop_draw_bounds(Widget *, void *, engine::Engine *);
} // namespace nfs::gui::widgets
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using gui::engine::Rectangle;

const char *__cdecl ticker_class_key() {
    return "MTicker";
}

static const char *__fastcall ticker_name(Widget *, void *) {
    return "MTicker";
}

static Dword __fastcall ticker_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(ticker_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall ticker_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "MTicker", 8) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static Ticker *construct_ticker(Ticker *ticker, void *parent) {
    construct_widget(ticker, parent);
    try {
        gui::construct_default_list(&ticker->pending);
        try {
            gui::construct_default_list(&ticker->active);
            try {
                ticker->timer_id = 0;
                ticker->base.base.methods = ticker_methods.data();
                ticker->accumulated_seconds = 0;
                ticker->maximum_messages = 10;
                ticker->spacing = 60;
                ticker->speed = 1.0f;
                ticker->timer_id = gui::create_widget_timer(ticker->base.base.id, 0.0333333350718021392822265625f);
            } catch (...) {
                gui::list_clear(&ticker->active);
                throw;
            }
        } catch (...) {
            gui::list_clear(&ticker->pending);
            throw;
        }
    } catch (...) {
        destroy_general(&ticker->base);
        throw;
    }
    return ticker;
}

static void destroy_ticker(Ticker *ticker) {
    ticker->base.base.methods = ticker_methods.data();
    gui::remove_widget_timer(ticker->timer_id);
    gui::list_clear(&ticker->active);
    gui::list_clear(&ticker->pending);
    destroy_general(&ticker->base);
}

static Ticker *__fastcall delete_ticker(Ticker *ticker, void *, Dword flags) {
    destroy_ticker(ticker);
    if (flags & 1)
        gui::release(ticker);
    return ticker;
}

static Dword __fastcall load_ticker(Ticker *ticker, void *, void *configuration) {
    gui::engine::pending::read_integer(configuration, "MTicker.mMaxMessages",
                                       &ticker->maximum_messages, 0);
    gui::engine::pending::read_integer(configuration, "MTicker.mSpacing", &ticker->spacing, 0);
    gui::engine::pending::read_float(configuration, "MTicker.mSpeed", &ticker->speed, 0);
    return gui::widgets::pending::load_general(&ticker->base, nullptr, configuration);
}

static Dword __fastcall save_ticker(Ticker *ticker, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "MTicker.mMaxMessages", ticker->maximum_messages,
                                    0);
    gui::engine::pending::write_int(configuration, "MTicker.mSpacing", ticker->spacing, 0);
    gui::engine::pending::write_float(configuration, "MTicker.mSpeed", ticker->speed, 2, 0);
    return gui::widgets::pending::save_general(&ticker->base, nullptr, configuration);
}
 
static void __fastcall initialize_ticker(Ticker *ticker, void *) {
    gui::widgets::pending::request_general_layout(&ticker->base, nullptr);
}

static void __fastcall ticker_timer(Ticker *ticker, void *, Dword id) {
    gui::timer_x87::ticker_timer(ticker, id);
}

static void __fastcall update_ticker(Ticker *ticker, void *, Dword ticks) {
    gui::timer_x87::update_ticker(ticker, ticks);
}

static void __fastcall push_ticker_bounds(Ticker *ticker, void *, gui::engine::Engine *render) {
    push_draw_bounds(&ticker->base.base, nullptr, render);
    const Rectangle *clip = invoke<const Rectangle *>(render, 66);
    const Rectangle *viewport = gui::engine::current_viewport_bounds(&render->base);
    Rectangle intersection;
    gui::engine::pending::intersect_rectangle(&intersection, clip, viewport);
    invoke(render, 64, &intersection);
    gui::engine::destroy_rectangle(&intersection);
}

static void __fastcall pop_ticker_bounds(Ticker *ticker, void *, gui::engine::Engine *render) {
    invoke(render, 65);
    pop_draw_bounds(&ticker->base.base, nullptr, render);
}

static void __fastcall draw_ticker(Ticker *ticker, void *, gui::engine::Engine *render) {
    const Rectangle *viewport = gui::engine::current_viewport_bounds(&render->base);
    invoke(render, 14, inherited_font_style(&ticker->base.base));
    for (Dword index = 0; index < gui::events::list_count(&ticker->active); ++index)
        if (auto *message =
                static_cast<TickerMessage *>(gui::events::list_value(&ticker->active, index))) {
            if ((message->flags & 2) && kernel::timer_frequency) {
                const int now = int(kernel::timer_count());
                const int frequency = int(kernel::timer_frequency);
                if (now % frequency < frequency / 2)
                    continue;
            }
            invoke(render, 42, &message->text,
                   int(Dword(viewport->width) - Dword(message->position.x) + Dword(viewport->left)),
                   int(Dword(message->position.y) + Dword(viewport->top)), &message->colour, Dword(0));
        }
}

const std::array<Method, 47> ticker_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_ticker), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::ticker_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::ticker_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::ticker_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_ticker), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_ticker), // 5.
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
    reinterpret_cast<Method>(::nfs::application::frontend::push_ticker_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_ticker), // 19.
    reinterpret_cast<Method>(::nfs::application::frontend::pop_ticker_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_ticker), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::ticker_timer), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_ticker), // 24.
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

static void *create_ticker(void *parent) {
    auto *storage = static_cast<Ticker *>(gui::allocate_flags(sizeof(Ticker), 0, "MTicker"));
    if (!storage)
        return nullptr;
    try {
        return construct_ticker(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord ticker_class;

static void unregister_ticker() {
    unregister_class(&ticker_class);
}

static void register_ticker() {
    register_class(&ticker_class, "MTicker", create_ticker, "GWidget");
    std::atexit(unregister_ticker);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_ticker, ".CRT$XCU_NFS_006900E8",
                       register_ticker);
} // namespace
} // namespace nfs::application::frontend
