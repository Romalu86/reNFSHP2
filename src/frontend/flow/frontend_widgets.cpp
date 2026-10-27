#include "nfs/core/application/application.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../gui/widgets/gui_widgets.hpp"
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::invoke;
extern Dword captured_widget_id;
void *create_widget_class(const char *, void *);
void *load_widget(const char *, const char *, void *);
Dword load_widget_children(void *, Dword);

void *find_child(void *widget, const char *name) {
    if (!name)
        return nullptr;
    const int count = at<int>(widget, 16);
    auto *children = reinterpret_cast<gui::Container *>(game::runtime::bytes(widget) + 8);
    for (int index = 0; index < count; ++index)
        if (void *child = gui::events::container_value(children, Dword(index))) {
            const auto *child_name =
                reinterpret_cast<const gui::String *>(game::runtime::bytes(child) + 72);
            if (gui::string_data(child_name) && !gui::string_compare_inline_ascii(child_name, name))
                return child;
        }
    return nullptr;
}

void set_widget_visible(void *widget, Dword visible) {
    const Dword captured = captured_widget_id;
    if (!visible && !(at<Dword>(widget, 60) & 2) && captured) {
        bool contains = captured == at<Dword>(widget, 4);
        if (!contains)
            for (void *ancestor = gui::events::find_widget(gui::context_state, captured);
                 ancestor;) {
                ancestor = at<void *>(ancestor, 56);
                if (ancestor == widget) {
                    contains = true;
                    break;
                }
            }
        if (contains) {
            captured_widget_id = 0;
            if (void *previous = gui::events::find_widget(gui::context_state, captured))
                gui::events::dispatch(previous, 13, 0, 0);
        }
    }
    at<Dword>(widget, 60) =
        (at<Dword>(widget, 60) & ~(visible ? Dword(2) : Dword(0))) | (visible ? Dword(0) : Dword(2));
}

void bring_widget_to_front(void *widget) {
    if (void *parent = at<void *>(widget, 56)) {
        const int count = at<int>(parent, 16);
        const int index = int(Dword(count) - 1);
        auto *ordered = reinterpret_cast<gui::Container *>(game::runtime::bytes(parent) + 24);
        void *last = count < index || index < 0
                         ? nullptr
                         : gui::events::container_value(ordered, Dword(index));
        if (last && last != widget) {
            at<Dword>(widget, 64) = at<Dword>(last, 64) + 1;
            gui::container_remove(ordered, widget);
            gui::container_sorted_insert(ordered, gui::widgets::compare_order,
                                         reinterpret_cast<void *>(at<Dword>(widget, 64)), widget,
                                         nullptr);
        }
    }
}

void *create_widget(const gui::String *resource, const gui::String *name, const gui::String *type,
                    void *parent) {
    if (!gui::active_engine())
        return nullptr;
    if (!resource->length || !name->length || !type->length) {
        void *widget = create_widget_class(gui::string_ascii(type), parent);
        if (widget) {
            gui::string_copy(reinterpret_cast<gui::String *>(game::runtime::bytes(widget) + 72),
                             name);
            gui::events::propagate(widget, 18, 0, 0, 0);
        }
        return widget;
    }
    gui::String separator, prefix, qualified;
    gui::string_construct(&separator);
    gui::string_construct(&prefix);
    gui::string_construct(&qualified);
    try {
        gui::string_assign(&separator, ".");
        gui::concatenate(&prefix, name, &separator);
        gui::concatenate(&qualified, &prefix, type);
    } catch (...) {
        gui::string_destruct(&qualified);
        gui::string_destruct(&prefix);
        gui::string_destruct(&separator);
        throw;
    }
    gui::string_destruct(&prefix);
    gui::string_destruct(&separator);
    void *widget;
    try {
        widget = load_widget(gui::string_ascii(resource), gui::string_ascii(&qualified), parent);
        if (widget) {
            const auto *filename = at<const gui::String *>(widget, 92);
            if (!filename)
                filename = &lifecycle::construction::unresolved::shared_string;
            if (filename->length)
                load_widget_children(widget, 1);
            gui::events::propagate(widget, 18, 0, 0, 0);
        }
    } catch (...) {
        gui::string_destruct(&qualified);
        throw;
    }
    gui::string_destruct(&qualified);
    return widget;
}

void set_menu_camera(Byte enabled) {
    for (void *director : game::tick_services) {
        if (!director)
            continue;
        const auto mode = [&] { return at<Dword>(director, 312 + at<Dword>(director, 356) * 4); };
        const auto pop = [&] {
            game::world::end_camera_mode(director, mode());
            --at<Dword>(director, 356);
            if (at<Dword>(director, 356))
                game::world::begin_camera_mode(director, mode());
            game::world::notify_camera_mode(director);
        };
        if (!enabled) {
            if (mode() == 8)
                pop();
        } else {
            if (invoke<void *>(director, 2) && at<Dword>(invoke<void *>(director, 2), 72) == 9)
                pop();
            if (mode() != 8) {
                if (at<Dword>(director, 356))
                    game::world::end_camera_mode(director, mode());
                if (at<int>(director, 356) < 10)
                    ++at<Dword>(director, 356);
                at<Dword>(director, 312 + at<Dword>(director, 356) * 4) = 8;
                game::world::begin_camera_mode(director, mode());
                game::world::notify_camera_mode(director);
            }
        }
    }
}
} // namespace nfs::application::pending
namespace nfs::gui::engine::pending {
using game::runtime::at;
using game::runtime::invoke;

void draw_widget_tree(void *widget, void *engine, Dword flags) {
    if (at<Dword>(widget, 60) & 2)
        return;
    Dword count = at<Dword>(widget, 16);
    const bool children = !(flags & 2) && count != 0;
    invoke(widget, 72 / 4, engine);
    auto *ordered = reinterpret_cast<Container *>(game::runtime::bytes(widget) + 24);
    if (children)
        for (Dword index = count; index;) {
            void *child = events::container_value(ordered, --index);
            if (child && (at<Dword>(child, 60) & 8192))
                draw_widget_tree(child, engine, flags);
        }
    if (!invoke<Dword>(engine, 280 / 4))
        invoke(widget, 76 / 4, engine);
    if (children && count) {
        const Dword cached_count = ordered->count;
        do {
            --count;
            Element *entry = count < cached_count ? ordered->elements[count] : nullptr;
            void *child = entry ? entry->value : nullptr;
            if (child && !(at<Dword>(child, 60) & 8192))
                draw_widget_tree(child, engine, flags);
        } while (count);
    }
    invoke(widget, 80 / 4, engine);
}

void draw_root(void *widget, void *engine, Dword flags) {
    if (engine) {
        if (flags & 1)
            invoke(widget, 68 / 4, engine);
        draw_widget_tree(widget, engine, flags);
    }
}
} // namespace nfs::gui::engine::pending
