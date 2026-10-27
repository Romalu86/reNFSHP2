#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../screens/frontend_menu_pages.hpp"
#include <cstdarg>
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;

Widget *find_menu_path(Widget *widget, Dword mode, const char *name, ...) {
    if (!widget || !name)
        return nullptr;
    va_list arguments;
    va_start(arguments, name);
    struct Arguments {
        va_list *value;
        ~Arguments() {
            va_end(*value);
        }
    } owner{&arguments};
    for (;;) {
        if (mode == 1) {
            Widget *found = nullptr;
            const int count = int(widget->children.count);
            for (int index = 0; index < count; ++index)
                if (auto *child = child_at(widget, index, 0))
                    if (gui::string_data(&child->name) &&
                        !gui::string_compare_ascii(&child->name, name)) {
                        found = child;
                        break;
                    }
            widget = found;
        } else if (mode == 2) {
            pending::LocalFrontendString target(name);
            auto *source = widget;
            Widget *found = nullptr;
            if (source->parent && target.value.length)
                for (int index = 0; index < int(source->parent->children.count); ++index)
                    if (auto *child = child_at(source->parent, index, 0))
                        if (child != source && gui::string_equal(&child->name, &target.value)) {
                            found = child;
                            break;
                        }
            widget = found;
        } else if (mode == 3) {
            widget = widget->parent;
            if (!widget || !gui::string_data(&widget->name) ||
                gui::string_compare_inline_ascii(&widget->name, name))
                return nullptr;
        } else
            return nullptr;
        if (!widget)
            return nullptr;
        mode = va_arg(arguments, Dword);
        if (!mode)
            return widget;
        name = va_arg(arguments, const char *);
        if (!name)
            return nullptr;
    }
}

static void accelerate_credits(void *credits) {
    at<Dword>(credits, 328) = at<Dword>(credits, 288);
}

static void reverse_credits(void *credits) {
    at<Dword>(credits, 328) = at<Dword>(credits, 296);
}

static void restore_credits_speed(void *credits) {
    at<Dword>(credits, 328) = at<Dword>(credits, 292);
}

static Dword __cdecl scroll_options_credits(void *sender, Dword, Dword key, Dword) {
    if (auto *credits = find_menu_path(static_cast<Widget *>(sender), 2, "<VIEW>", Dword(1),
                                       "Credits", Dword(1), "<credits>", Dword(0))) {
        if (key == 208)
            accelerate_credits(credits);
        else if (key == 200)
            reverse_credits(credits);
    }
    return 0;
}

static Dword __cdecl release_options_credits(void *sender, Dword, Dword key, Dword) {
    if (auto *credits = find_menu_path(static_cast<Widget *>(sender), 2, "<VIEW>", Dword(1),
                                       "Credits", Dword(1), "<credits>", Dword(0)))
        if (key == 208 || key == 200)
            restore_credits_speed(credits);
    return 0;
}
static CallbackRecord options_callbacks[3];

static void unregister_options_callbacks() {
    for (unsigned index = 3; index; --index)
        unregister_callback(options_callbacks + index - 1);
}

static void register_options_callbacks() {
    register_callback(options_callbacks, general_class_name, "GEVENT_FOCUSSET", 14,
                      "OptionsScreen::UpdateDisplayData", update_options_heading, 0);
    register_callback(options_callbacks + 1, general_class_name, "GEVENT_KEYDOWN", 6,
                      "Options_ScrollCredits", scroll_options_credits, 0);
    register_callback(options_callbacks + 2, general_class_name, "GEVENT_KEYUP", 8,
                      "Options_RestoreScrollCredits", release_options_credits, 0);
    std::atexit(unregister_options_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_options_callbacks, ".CRT$XCU_NFS_006900F8",
                       register_options_callbacks);
} // namespace
} // namespace nfs::application::frontend
