#include "nfs/gui/events/gui_events.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::gui::events {
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
Node *node(Object *value) {
    return reinterpret_cast<Node *>(value);
}
} // namespace

int compare_widget_id(const void *widget, const void *key) {
    const Dword id = reinterpret_cast<const Dword *>(widget)[1];
    const Dword sought = reinterpret_cast<Dword>(key);
    if (id < sought)
        return -1;
    return sought < id;
}

void *find_widget(ContextState *context, Dword id) {
    if (!id)
        return nullptr;
    Guard guard(context->lock);
    return container_find(&context->third, compare_widget_id, reinterpret_cast<const void *>(id));
}

List *construct_list(List *list, Dword flags) {
    list->methods = list_methods;
    void *lock = host->create_lock();
    list->first = nullptr;
    list->last = nullptr;
    list->count = 0;
    list->granularity = flags;
    list->lock = lock;
    return list;
}

void destroy_node(Node *item) {
    item->methods = node_methods;
    if (item->value && item->dispose)
        item->dispose(item->value);
    Node *next = item->next, *previous = item->previous;
    if (next && previous) {
        previous->next = next;
        next->previous = previous;
    } else if (!previous) {
        if (next)
            next->previous = nullptr;
    } else
        previous->next = nullptr;
    item->methods = base_methods;
}

void *__fastcall delete_node(Node *item, void *, unsigned flags) {
    destroy_node(item);
    if (flags & 1)
        release(item);
    return item;
}
const Destroy node_methods[]{reinterpret_cast<Destroy>(delete_node)};

Dword push_front(List *list, void *value, Dispose dispose) {
    Guard guard(list->lock);
    auto *item = static_cast<Node *>(allocate_flags(sizeof(Node), list->granularity, "GQElement"));
    if (item) {
        item->value = value;
        item->methods = node_methods;
        item->dispose = dispose;
        item->previous = nullptr;
        item->next = node(list->first);
        if (item->next)
            item->next->previous = item;
    }
    list->first = reinterpret_cast<Object *>(item);
    if (!list->last)
        list->last = reinterpret_cast<Object *>(item);
    if (!item)
        return 1;
    ++list->count;
    return 0;
}

Dword push_back(List *list, void *value, Dispose dispose) {
    Guard guard(list->lock);
    auto *item = static_cast<Node *>(allocate_flags(sizeof(Node), list->granularity, "GQElement"));
    if (item) {
        item->value = value;
        item->methods = node_methods;
        item->dispose = dispose;
        item->next = nullptr;
        item->previous = node(list->last);
        if (item->previous)
            item->previous->next = item;
    }
    list->last = reinterpret_cast<Object *>(item);
    if (!list->first)
        list->first = reinterpret_cast<Object *>(item);
    if (!item)
        return 1;
    ++list->count;
    return 0;
}

void *pop_front(List *list) {
    Guard guard(list->lock);
    Node *item = node(list->first);
    if (!item)
        return nullptr;
    void *value = item->value;
    item->dispose = nullptr;
    item->value = nullptr;
    Node *next = item->next;
    item->methods[0](item, 1);
    list->first = reinterpret_cast<Object *>(next);
    if (!next)
        list->last = nullptr;
    --list->count;
    return value;
}

void *peek_front(List *list) {
    Guard guard(list->lock);
    return list->first ? node(list->first)->value : nullptr;
}

void *list_value(List *list, Dword index) {
    Guard guard(list->lock);
    if (index >= list->count)
        return nullptr;
    for (Node *item = node(list->first); item; item = item->next)
        if (!index--)
            return item->value;
    return nullptr;
}

void *peek_back(List *list) {
    Guard guard(list->lock);
    return list->last ? node(list->last)->value : nullptr;
}

Dword contains(List *list, const void *value) {
    Guard guard(list->lock);
    for (Node *item = node(list->first); item; item = item->next)
        if (item->value == value)
            return 1;
    return 0;
}

Dword remove_value(List *list, void *value) {
    Guard guard(list->lock);
    if (!value) return 2;
    for (Node *item=node(list->first);item;item=item->next)
        if (item->value==value) {
            if (list->first==reinterpret_cast<Object *>(item)) list->first=reinterpret_cast<Object *>(item->next);
            if (list->last==reinterpret_cast<Object *>(item)) list->last=reinterpret_cast<Object *>(item->previous);
            item->dispose=nullptr;
            item->methods[0](item,1); --list->count; return 0;
        }
    return 6;
}

void remove_if(List *list, Predicate predicate, Dword context) {
    Guard guard(list->lock);
    Node *item = node(list->first);
    while (item) {
        Node *current = item;
        item = current->next;
        if (predicate(current->value, context)) {
            if (current == node(list->first))
                list->first = reinterpret_cast<Object *>(current->next);
            if (current == node(list->last))
                list->last = reinterpret_cast<Object *>(current->previous);
            current->methods[0](current, 1);
            --list->count;
        }
    }
}

void destroy_message(Message *message) {
    message->methods = base_methods;
}

void *__fastcall delete_message(Message *message, void *, unsigned flags) {
    destroy_message(message);
    if (flags & 1)
        release(message);
    return message;
}
const Destroy message_methods[]{reinterpret_cast<Destroy>(delete_message)};

Dword post(ContextState *context, Dword target, Dword event, Dword first, Dword second, Dword front) {
    auto *message = static_cast<Message *>(allocate_flags(sizeof(Message), 256, "GMsg"));
    if (!message)
        return 1;
    message->target = target;
    message->second = second;
    message->event = event;
    message->methods = message_methods;
    message->first = first;
    Dword result = front == 1 ? push_front(&context->index, message, dispose_object)
                             : push_back(&context->index, message, dispose_object);
    if (result)
        message->methods[0](message, 1);
    return result;
}

Dword event_matches(void *message, Dword target) {
    return reinterpret_cast<Message *>(message)->event == target;
}
// Predicate really compares the event word at +4.
void remove_event(ContextState *context, Dword event) {
    Guard guard(context->lock);
    remove_if(&context->index, event_matches, event);
}
} // namespace nfs::gui::events

namespace nfs::gui::events {
// Clear nodes without destroying the list lock.
Dword clear_list(List *list) {
    Guard guard(list->lock);
    Object *item = list->first;
    while (item) {
        Object *next = reinterpret_cast<Object **>(item)[3];
        destroy(item);
        item = next;
    }
    list->first = list->last = nullptr;
    list->count = 0;
    return 0;
}
// The saved count load follows lock release.
Dword list_count(List *list) {
    { Guard guard(list->lock); }
    return list->count;
}
// Unlike remove_value, the node retains its value disposer.
Dword erase_value(List *list, void *value) {
    Guard guard(list->lock);
    if (!value)
        return 2;
    for (Node *item = node(list->first); item; item = item->next) {
        if (item->value != value)
            continue;
        if (list->first == reinterpret_cast<Object *>(item))
            list->first = reinterpret_cast<Object *>(item->next);
        if (list->last == reinterpret_cast<Object *>(item))
            list->last = reinterpret_cast<Object *>(item->previous);
        item->methods[0](item, 1);
        --list->count;
        return 0;
    }
    return 6;
}

Dword take_target(ContextState *context, Dword target, Dword *event, Dword *first, Dword *second,
                 int minimum, int maximum, Dword remove) {
    if (list_count(&context->index)) {
        Dword index = 0;
        do {
            auto *message = static_cast<Message *>(list_value(&context->index, index));
            if (message && message->target == target && minimum <= int(message->event) &&
                int(message->event) <= maximum) {
                *event = message->event;
                *first = message->first;
                *second = message->second;
                if (remove)
                    erase_value(&context->index, message);
                return 1;
            }
            ++index;
        } while (index < list_count(&context->index));
    }
    return 0;
}
}
