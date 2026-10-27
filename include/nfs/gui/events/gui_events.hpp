#pragma once
#include "nfs/gui/lifecycle/gui_context.hpp"
namespace nfs::gui::events {
struct Node {
    const Destroy *methods;
    void *value;
    Dispose dispose;
    Node *next, *previous;
};
struct Message {
    const Destroy *methods;
    Dword event, first, second, target;
};
static_assert(sizeof(Node) == 20 && sizeof(Message) == 20);
extern const Destroy node_methods[], message_methods[];
List *construct_list(List *, Dword);
Dword push_front(List *, void *, Dispose);
Dword clear_list(List *);
Dword list_count(List *);
Dword push_back(List *, void *, Dispose);
void *pop_front(List *);
void *peek_front(List *);
void *peek_back(List *);
void *list_value(List *, Dword);
Dword contains(List *, const void *);
Dword remove_value(List *, void *);
Dword erase_value(List *, void *);
Dword take_target(ContextState *, Dword, Dword *, Dword *, Dword *, int, int, Dword);
using Predicate = Dword(__cdecl *)(void *, Dword);
void remove_if(List *, Predicate, Dword);
void destroy_node(Node *);
void destroy_message(Message *);
Dword post(ContextState *, Dword, Dword, Dword, Dword, Dword);
void remove_event(ContextState *, Dword);
Dword event_matches(void *, Dword);
void *container_value(Container *, Dword);
Dword call_handlers(Container *, void *, Dword, Dword, Dword);
void notify_handler(Container *, void *, Dword);
Dword dispatch(void *, Dword, Dword, Dword);
void propagate(void *, Dword, Dword, Dword, Dword);
void post_widget(void *, Dword, Dword, Dword);
void request_focus(void *, Dword);
void notify_context_focus(ContextState *, void *, Dword);
void *focused(ContextState *);
Dword contains_focus(ContextState *, void *);
void *focus_ancestor(void *);
Dword is_within_focus(void *);
} // namespace nfs::gui::events
