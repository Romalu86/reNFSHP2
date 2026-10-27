#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::gui {
struct FocusList {
    const Destroy *methods;
    List list;
};
struct ContextState {
    const Destroy *methods;
    Dword references;
    Container first, second;
    List index;
    Container third;
    Dword value_50;
    void *lock;
    Dword value_58;
    FocusList *extra;
    Container fourth;
};
static_assert(sizeof(ContextState) == 112 && sizeof(FocusList) == 28 &&
              offsetof(FocusList, list) == 4 && offsetof(ContextState, extra) == 92);
extern ContextState *context_state;
extern const Destroy list_methods[];
void list_clear(List *);
List *construct_default_list(List *);
} // namespace nfs::gui
