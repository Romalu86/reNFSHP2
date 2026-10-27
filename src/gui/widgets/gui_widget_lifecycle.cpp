#include "gui_widgets.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::gui::engine::pending {
ini::Document *acquire_configuration_document(const String *);
void release_configuration_document(const String *);
} // namespace nfs::gui::engine::pending
namespace nfs::application::pending {
kernel::Dword captured_widget_id{};
}
namespace nfs::gui::widgets {
using game::runtime::invoke;
Dword active_widget_id{};
static Dword widget_serial = 1;
namespace {
struct Guard {
    void *lock;
    explicit Guard(void *value) : lock(value) {
        if (lock)
            host->enter(lock);
    }
    ~Guard() {
        if (lock)
            host->leave(lock);
    }
};
const String *empty_string() {
    return &lifecycle::construction::unresolved::shared_string;
}
Dword extract_child(Container *container, void *child) {
    if (!child)
        return 2;
    const Dword count = container->count;
    for (Dword index = 0; index < count; ++index)
        if (Element *element = container->elements[index])
            if (element->value == child) {
                element->dispose = nullptr;
                return container_remove_at(container, index);
            }
    return 6;
}
} // namespace

int compare_order(const void *widget, const void *key) {
    return int(reinterpret_cast<Dword>(key)) <= static_cast<const Widget *>(widget)->z_order ? 1
                                                                                            : -1;
}

int compare_tab(const void *widget, const void *key) {
    return int(reinterpret_cast<Dword>(key)) <= static_cast<const Widget *>(widget)->tab_index ? 1
                                                                                              : -1;
}

int compare_id(const void *widget, const void *key) {
    const Dword id = static_cast<const Widget *>(widget)->id, wanted = reinterpret_cast<Dword>(key);
    return id < wanted ? -1 : wanted < id;
}
namespace {

void dispose_child(void *storage) {
    auto *child = static_cast<Widget *>(storage);
    events::propagate(child, 20, 0, 0, 1);
    container_clear(&child->draw_order);
    container_clear(&child->tab_order);
    container_clear(&child->children);
    if (child)
        invoke(child, 0, Dword(1));
}
} // namespace

Dword register_widget(ContextState *context, Widget *widget) {
    if (container_append(&context->third, widget, nullptr))
        return 0;
    const Dword id = widget_serial;
    context->value_58 = 1;
    widget_serial += 4;
    return id;
}

Dword unregister_widget(ContextState *context, Widget *widget) {
    Guard guard(context->lock);
    context->value_58 = 1;
    if (widget) {
        const int index = int(container_find_index(&context->fourth, events::compare_widget_id,
                                                   reinterpret_cast<void *>(widget->id)));
        if (index >= 0)
            container_remove_at(&context->fourth, Dword(index));
    }
    const int index = int(container_find_index(&context->third, events::compare_widget_id,
                                               reinterpret_cast<void *>(widget->id)));
    return index >= 0 ? container_remove_at(&context->third, Dword(index)) : 6;
}

Dword attach_child(Widget *parent, Widget *child) {
    if (parent == child)
        return 4;
    if (!child)
        return 2;
    if (child->parent) {
        container_remove(&child->parent->draw_order, child);
        container_remove(&child->parent->tab_order, child);
        if (Dword result = extract_child(&child->parent->children, child))
            return result;
    }
    Dword result = container_sorted_insert(
        &parent->children, compare_id, reinterpret_cast<void *>(child->id), child, dispose_child);
    if (!result) {
        result =
            container_sorted_insert(&parent->draw_order, compare_order,
                                    reinterpret_cast<void *>(Dword(child->z_order)), child, nullptr);
        if (!result) {
            result = container_sorted_insert(&parent->tab_order, compare_tab,
                                             reinterpret_cast<void *>(Dword(child->tab_index)),
                                             child, nullptr);
            if (!result)
                child->parent = parent;
        }
    }
    return result;
}

void *descendant(Widget *parent, Dword id) {
    auto *child = id ? static_cast<Widget *>(events::find_widget(context_state, id)) : nullptr;
    Widget *current = child;
    do {
        if (!current)
            return nullptr;
        current = current->parent;
    } while (current != parent);
    return child;
}

void *construct_base(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(storage);
    widget->methods = reinterpret_cast<const Method *>(gui::base_methods);
    widget->id = 0;
    int completed = 0;
    try {
        container_construct(&widget->children);
        completed = 1;
        container_construct(&widget->draw_order);
        completed = 2;
        container_construct(&widget->tab_order);
        completed = 3;
        widget->parent = nullptr;
        widget->flags = 0;
        widget->z_order = widget->tab_index = 0;
        try {
            string_construct_copy(&widget->name, empty_string());
        } catch (...) {
            widget->name.methods = gui::base_methods;
            throw;
        }
        completed = 4;
        widget->resource = nullptr;
        engine::construct_rectangle(&widget->bounds);
        completed = 5;
        engine::construct_point(&widget->scroll);
        completed = 6;
        widget->layout_flags = widget->value_84 = 0;
        widget->colour = Dword(-1);
        try {
            string_construct_copy(&widget->text, empty_string());
        } catch (...) {
            widget->text.methods = gui::base_methods;
            throw;
        }
        completed = 7;
        widget->font_style = widget->palette = 0;
        engine::construct_rectangle(&widget->clip);
        completed = 8;
        engine::construct_rectangle(&widget->padding);
        completed = 9;
        widget->modal_result = 0;
        widget->value_d0 = 0;
        widget->handlers = nullptr;
        widget->value_d8 = widget->value_e0 = 0;
        string_construct(&widget->tooltip);
        completed = 10;
        widget->timer = widget->options = 0;
        widget->methods = widget_methods;
        widget->id = register_widget(context_state, widget);
        if (parent)
            attach_child(static_cast<Widget *>(parent), widget);
        else if (widget->parent) {
            container_remove(&widget->parent->draw_order, widget);
            container_remove(&widget->parent->tab_order, widget);
            extract_child(&widget->parent->children, widget);
            widget->parent = nullptr;
        }
        return widget;
    } catch (...) {
        if (completed >= 10)
            string_destruct(&widget->tooltip);
        if (completed >= 9)
            engine::destroy_rectangle(&widget->padding);
        if (completed >= 8)
            engine::destroy_rectangle(&widget->clip);
        if (completed >= 7)
            string_destruct(&widget->text);
        if (completed >= 6)
            engine::destroy_point(&widget->scroll);
        if (completed >= 5)
            engine::destroy_rectangle(&widget->bounds);
        if (completed >= 4)
            string_destruct(&widget->name);
        if (completed >= 3)
            container_destruct(&widget->tab_order);
        if (completed >= 2)
            container_destruct(&widget->draw_order);
        if (completed >= 1)
            container_destruct(&widget->children);
        widget->methods = reinterpret_cast<const Method *>(gui::base_methods);
        throw;
    }
}

void destroy_base(Widget *widget) {
    widget->methods = widget_methods;
    if (widget->handlers) {
        destroy(reinterpret_cast<Object *>(widget->handlers));
        widget->handlers = nullptr;
    }
    const auto release_active = [&](Dword &id, Dword event) {
        const Dword previous = id;
        if (previous && (previous == widget->id || descendant(widget, previous))) {
            id = 0;
            if (void *active = events::find_widget(context_state, previous))
                events::dispatch(active, event, 0, 0);
        }
    };
    release_active(active_widget_id, 15);
    release_active(application::pending::captured_widget_id, 13);
    auto *previous_resource = widget->resource;
    const String *resource = previous_resource ? &previous_resource->string : empty_string();
    if (!string_equal(resource, empty_string())) {
        if (widget->options & 16) {
            engine::pending::release_configuration_document(resource);
            widget->options &= ~Dword(16);
        }
        string_release(previous_resource);
        auto *replacement_resource = string_acquire(string_ascii(empty_string()));
        widget->resource = replacement_resource;
        if (container_mode) {
            engine::pending::acquire_configuration_document(
                replacement_resource ? &replacement_resource->string : empty_string());
            widget->options |= 16;
        }
    }
    string_copy(&widget->name, empty_string());
    if (widget->parent)
        widget->parent = nullptr;
    unregister_widget(context_state, widget);
    string_destruct(&widget->tooltip);
    engine::destroy_rectangle(&widget->padding);
    engine::destroy_rectangle(&widget->clip);
    string_destruct(&widget->text);
    engine::destroy_point(&widget->scroll);
    engine::destroy_rectangle(&widget->bounds);
    string_destruct(&widget->name);
    container_destruct(&widget->tab_order);
    container_destruct(&widget->draw_order);
    container_destruct(&widget->children);
    widget->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

void *construct_widget(void *storage, void *parent) {
    construct_base(storage, parent);
    auto *widget = static_cast<GeneralWidget *>(storage);
    widget->base.flags |= 4;
    widget->base.methods = general_methods;
    widget->style = 0;
    widget->state = 31;
    return widget;
}

void destroy_general(GeneralWidget *widget) {
    widget->base.methods = general_methods;
    destroy_base(&widget->base);
}
} // namespace nfs::gui::widgets
