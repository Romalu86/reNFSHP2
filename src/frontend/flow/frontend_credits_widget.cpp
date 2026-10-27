#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../screens/frontend_menu_pages.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdlib>
#include <cstring>
#include <cmath>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using kernel::numeric::Extended;
struct CreditsFont {
    const gui::Destroy *methods;
    Dword handle;
};

static CreditsFont *__fastcall delete_credits_font(CreditsFont *font, void *, Dword flags) {
    font->methods = gui::base_methods;
    if (flags & 1)
        gui::release(font);
    return font;
}
static const gui::Destroy credits_font_methods[] = {
    reinterpret_cast<gui::Destroy>(delete_credits_font)};
struct Credits {
    GeneralWidget base;
    Dword timer;
    int font_height, line_height, scroll_offset, credit_count, last_credit;
    int fast_speed, normal_speed, reverse_speed;
    float initial_delay;
    Dword title_colour;
    CreditsFont title_font, body_font;
    float delay;
    int scroll_speed;
    float delay_step;
};
static_assert(sizeof(Credits) == 336 && offsetof(Credits, body_font) == 316);
extern const std::array<Method, 47> credits_methods;

static const char *__cdecl credits_class_key() {
    return "HP2Credits";
}

static const char *__fastcall credits_name(Widget *, void *) {
    return "HP2Credits";
}

static Dword __fastcall credits_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(credits_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall credits_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2Credits", 11) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static Credits *construct_credits(Credits *credits, void *parent) {
    construct_widget(credits, parent);
    credits->timer = 0;
    credits->font_height = credits->line_height = credits->scroll_offset = credits->credit_count =
        credits->last_credit = 0;
    credits->fast_speed = credits->normal_speed = 0;
    credits->initial_delay = 0;
    credits->title_colour = 0;
    credits->title_font = {credits_font_methods, 0};
    credits->body_font = {credits_font_methods, 0};
    credits->delay = 0;
    credits->scroll_speed = 1;
    credits->delay_step = 0.0333333350718021392822265625f;
    credits->base.base.methods = credits_methods.data();
    try {
        credits->timer = gui::create_widget_timer(credits->base.base.id, credits->delay_step);
    } catch (...) {
        credits->body_font.methods = credits->title_font.methods = gui::base_methods;
        destroy_general(&credits->base);
        throw;
    }
    return credits;
}

static void destroy_credits(Credits *credits) {
    credits->base.base.methods = credits_methods.data();
    gui::remove_widget_timer(credits->timer);
    credits->body_font.methods = credits->title_font.methods = gui::base_methods;
    destroy_general(&credits->base);
}

static Credits *__fastcall delete_credits(Credits *credits, void *, Dword flags) {
    destroy_credits(credits);
    if (flags & 1)
        gui::release(credits);
    return credits;
}

static int visible_credit_lines(Credits *credits) {
    return credits->line_height ? credits->base.base.bounds.height / credits->line_height + 1 : 0;
}
static const char *credit_symbol(const gui::String *text) {
    if (text->reference)
        return gui::string_resolver ? invoke<const char *>(gui::string_resolver, 5, text->reference)
                                    : nullptr;
    return text->wide ? nullptr : static_cast<const char *>(text->data);
}
static bool title_credit(Credits *credits, int index) {
    int reference = int(Dword(index) + 2081);
    if (reference < 2081 || reference >= credits->last_credit)
        reference = 0;
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, Dword(reference));
    if (!gui::engine::pending::character_at(&text.value, 0))
        return false;
    const char *symbol = credit_symbol(&text.value);
    return symbol && std::strstr(symbol, "kTxttit");
}

static Dword *credit_colour(Credits *credits, Dword *colour, int index) {
    *colour = title_credit(credits, index) ? credits->title_colour : credits->base.base.colour;
    return colour;
}

static CreditsFont *credit_font(Credits *credits, CreditsFont *font, int index) {
    font->methods = credits_font_methods;
    font->handle = 0;
    const CreditsFont *source =
        title_credit(credits, index) ? &credits->title_font : &credits->body_font;
    if (source != font)
        font->handle = source->handle;
    return font;
}

static void __fastcall initialize_credits(Credits *credits, void *) {
    gui::widgets::pending::request_general_layout(&credits->base, nullptr);
    credits->credit_count = int(Dword(credits->last_credit) - 2081);
    auto *engine = gui::active_engine();
    credits->body_font.handle = inherited_font_style(&credits->base.base);
    invoke(engine, 14, credits->body_font.handle);
    credits->font_height = invoke<int>(engine, 17);
    credits->delay = credits->initial_delay;
    credits->line_height = int(Dword(credits->font_height / 3) + Dword(credits->font_height));
}

static void __fastcall update_credits(Credits *credits, void *, Dword) {
    const int first = credits->line_height ? credits->scroll_offset / credits->line_height : 0;
    const int visible = visible_credit_lines(credits);
    if (int(Dword(first) + Dword(visible)) >
        int(Dword(credits->credit_count) + Dword(visible_credit_lines(credits))))
        credits->scroll_offset =
            int(Dword(0) - Dword(credits->line_height) * Dword(visible_credit_lines(credits)));
}

static void __fastcall tick_credits(Credits *credits, void *, Dword timer) {
    if (timer != credits->timer)
        return;
    if (credits->delay != 0.0f) {
        const auto format = kernel::numeric::current_format();
        const auto next_delay =
            Extended(credits->delay, format) - Extended(credits->delay_step, format);
        credits->delay = float(next_delay);
        if (!(Extended(0, format) < next_delay))
            credits->delay = 0;
        return;
    }
    credits->scroll_offset = int(Dword(credits->scroll_offset) + Dword(credits->scroll_speed));
    if (credits->scroll_speed < 0 && credits->scroll_offset <= 0)
        credits->scroll_offset = 0;
}

static Dword __fastcall credits_pointer_state(Credits *credits, void *, Dword first, Dword second) {
    if (second & 4194304) {
        credits->delay = 1;
        capture_widget(&credits->base.base);
    }
    return style_result_38(&credits->base.base, nullptr, first, second);
}

static Dword __fastcall drag_credits(Credits *credits, void *, const gui::engine::Point *delta,
                                    Dword second) {
    if (pending::captured_widget_id == credits->base.base.id) {
        credits->scroll_offset = int(Dword(credits->scroll_offset) + Dword(delta->y));
        credits->delay = 1;
        if (credits->scroll_offset <= 0)
            credits->scroll_offset = 0;
    }
    return gui::widgets::pending::update_active_style(&credits->base.base, nullptr,
                                                      reinterpret_cast<Dword>(delta), second);
}

static Dword __fastcall release_credits_pointer(Credits *credits, void *, Dword first, Dword second) {
    if (pending::captured_widget_id == credits->base.base.id) {
        release_capture(&credits->base.base);
        credits->delay = 0;
    }
    return capture_style(&credits->base.base, nullptr, first, second);
}

static Dword __fastcall scroll_credits_line(Credits *credits, void *, Dword forward, Dword) {
    if (!forward) {
        credits->scroll_offset = int(Dword(credits->scroll_offset) - Dword(credits->line_height));
        if (credits->scroll_offset <= 0)
            credits->scroll_offset = 0;
    } else
        credits->scroll_offset = int(Dword(credits->scroll_offset) + Dword(credits->line_height));
    return 0;
}

static void __fastcall draw_credits(Credits *credits, void *, gui::engine::Engine *engine) {
    if (!credits->line_height)
        return;
    invoke(engine, 14, credits->body_font.handle);
    invoke(engine, 21, family::inherited_palette(&credits->base.base));
    gui::engine::Rectangle clip = *gui::engine::current_viewport_bounds(&engine->base);
    clip.width = gui::engine::source_bounds.width;
    clip.left = 0;
    invoke(engine, 64, &clip);
    int y = int(Dword(clip.top) - Dword(credits->scroll_offset % credits->line_height));
    int index = credits->scroll_offset / credits->line_height;
    int reference = int(Dword(index) + 2081);
    for (;;) {
        const int first = credits->line_height ? credits->scroll_offset / credits->line_height : 0;
        if (index > int(Dword(first) + Dword(visible_credit_lines(credits))))
            break;
        if (reference >= 2081 && reference < credits->last_credit && reference) {
            Dword colour;
            credit_colour(credits, &colour, index);
            CreditsFont font;
            credit_font(credits, &font, index);
            invoke(engine, 14, font.handle);
            font.methods = gui::base_methods;
            gui::engine::Rectangle extent{gui::engine::rectangle_methods, 0, 0, 0, 0};
            Dword measured;
            {
                pending::LocalFrontendString text;
                gui::string_reference(&text.value, Dword(reference));
                measured = invoke<Dword>(engine, 15, &text.value, &extent);
            }
            if (measured) {
                pending::LocalFrontendString text;
                gui::string_reference(&text.value, Dword(reference));
                const int centre = int(Dword(clip.left) + Dword(clip.width / 2));
                invoke(engine, 42, &text.value, int(Dword(centre) - Dword(extent.width / 2)), y,
                       &colour, Dword(0));
            }
        }
        y = int(Dword(y) + Dword(credits->line_height));
        index = int(Dword(index) + 1);
        reference = int(Dword(reference) + 1);
    }
    invoke(engine, 65);
}

static void read_credits_font(void *configuration, const char *key, CreditsFont *font) {
    const gui::String *value = invoke<const gui::String *>(configuration, 5, key);
    font->handle = 0;
    if (value->length)
        if (auto *engine = gui::active_engine())
            font->handle = invoke<Dword>(engine, 7, value);
}

static Dword write_credits_font(void *configuration, const char *key, const CreditsFont *font) {
    pending::LocalFrontendString value;
    bool remove;
    if (auto *engine = gui::active_engine(); engine && font->handle) {
        gui::String name;
        invoke<gui::String *>(engine, 8, &name, font->handle);
        const char *source = gui::string_ascii(&name);
        remove = source == nullptr;
        try {
            gui::string_assign(&value.value, source);
        } catch (...) {
            gui::string_destruct(&name);
            throw;
        }
        gui::string_destruct(&name);
    } else {
        const auto *shared = &lifecycle::construction::unresolved::shared_string;
        pending::LocalFrontendString name(shared);
        const char *source = gui::string_ascii(&name.value);
        remove = source == nullptr;
        gui::string_assign(&value.value, source);
    }
    return invoke<Dword>(configuration, 7, key, &value.value, "FONT", Dword(0), Dword(remove));
}

static Dword copy_credit_setting(void *configuration, const char *key, char *out, Dword count,
                                const char *fallback) {
    if (!out)
        return 0;
    pending::LocalFrontendString value;
    if (invoke<Dword>(configuration, 8, key,
                     &lifecycle::construction::unresolved::shared_string, &value.value)) {
        std::strncpy(out, gui::string_ascii(&value.value), count);
        return int(value.value.length) <= int(count) ? Dword(value.value.length) : count;
    }
    if (fallback)
        std::strncpy(out, fallback, count);
    else if (count)
        *out = 0;
    return 0;
}

static Dword __fastcall load_credits(Credits *credits, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_float(configuration, "HP2Credits.mInitialDelay", &credits->initial_delay, 0);
    read_int(configuration, "HP2Credits.mDelta", &credits->normal_speed, 0);
    read_int(configuration, "HP2Credits.mFFwd", &credits->fast_speed, 0);
    read_int(configuration, "HP2Credits.mRev", &credits->reverse_speed, 0);
    const Dword fallback = 0;
    read_colour(configuration, "HP2Credits.mTitleColor", &credits->title_colour, &fallback);
    read_credits_font(configuration, "HP2Credits.mTitleFont", &credits->title_font);
    char token[80]{};
    copy_credit_setting(configuration, "HP2Credits.mLastCreditID", token, 79, nullptr);
    game::world::pending::resolve_text_token(token,
                                             reinterpret_cast<Dword *>(&credits->last_credit));
    if (credits->last_credit < 0)
        credits->last_credit = 0;
    return gui::widgets::pending::load_general(&credits->base, nullptr, configuration);
}

static Dword __fastcall save_credits(Credits *credits, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_float(configuration, "HP2Credits.mInitialDelay", credits->initial_delay, 2, 0);
    write_int(configuration, "HP2Credits.mDelta", credits->normal_speed, 0);
    write_int(configuration, "HP2Credits.mFFwd", credits->fast_speed, 0);
    write_int(configuration, "HP2Credits.mRev", credits->reverse_speed, 0);
    write_int(configuration, "HP2Credits.mLastCredit", credits->last_credit, 0);
    const Dword fallback = 0;
    write_colour(configuration, "HP2Credits.mTitleColor", &credits->title_colour, &fallback);
    write_credits_font(configuration, "HP2Credits.mTitleFont", &credits->title_font);

    return gui::widgets::pending::load_general(&credits->base, nullptr, configuration);
}

const std::array<Method, 47> credits_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_credits), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::credits_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::credits_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::credits_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_credits), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_credits), // 5.
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
    reinterpret_cast<Method>(::nfs::application::frontend::draw_credits), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_credits), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_credits), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_credits), // 24.
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
    reinterpret_cast<Method>(::nfs::application::frontend::credits_pointer_state), // 38.
    reinterpret_cast<Method>(::nfs::application::frontend::drag_credits), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::application::frontend::release_credits_pointer), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::application::frontend::scroll_credits_line), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

static void *create_credits(void *parent) {
    auto *credits = static_cast<Credits *>(gui::allocate_flags(sizeof(Credits), 0, "HP2Credits"));
    if (!credits)
        return nullptr;
    try {
        return construct_credits(credits, parent);
    } catch (...) {
        gui::release_flags(credits, 0, "HP2Credits");
        throw;
    }
}
static ClassRecord credits_class;

static void unregister_credits() {
    unregister_class(&credits_class);
}

static void register_credits() {
    register_class(&credits_class, "HP2Credits", create_credits, "GWidget");
    std::atexit(unregister_credits);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_credits, ".CRT$XCU_NFS_006900A4",
                       register_credits);
} // namespace
} // namespace nfs::application::frontend
