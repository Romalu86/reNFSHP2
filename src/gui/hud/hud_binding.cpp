#include "hud_instruments.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::application::pending {
void release_result_allocation(void *);
}
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::system;

static Binding *__fastcall delete_binding(Binding *binding, void *, Dword flags) {
    binding->methods = binding_methods;
    if (flags & 1) pending::release_result_allocation(binding);
    return binding;
}
const Method binding_methods[1] = {reinterpret_cast<Method>(delete_binding)};

Binding *construct_binding(Binding *binding, Widget *owner) {
    binding->owner = owner;
    binding->methods = binding_methods;
    return binding;
}

void destroy_binding(Binding *binding) { binding->methods = binding_methods; }

Dword binding_rule(Binding *, Dword rule) { return network::packets::rule_value(system(72), int(rule)); }
static View *binding_view(Binding *binding) {
    if (!binding->owner) return nullptr;
    pending::LocalFrontendString type("HudView");
    auto *view = find_menu_ancestor(binding->owner, &type.value,
        &lifecycle::construction::unresolved::shared_string, 0);
    return view && game::runtime::invoke<Dword>(view, 2, view_class_key)
        ? reinterpret_cast<View *>(view) : nullptr;
}

void *binding_camera(Binding *binding) {
    View *view = binding_view(binding);
    return view ? game::tick_services[view->second_player != 0] : nullptr;
}

void *binding_car(Binding *binding) {
    View *view = binding_view(binding);
    return view ? game::runtime::physics::local_car(system(68), view->second_player != 0) : nullptr;
}
}
