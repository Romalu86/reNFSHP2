#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "gui_script.hpp"
#include <cstdlib>
namespace nfs::gui::widgets::script {
static FunctionRecord builtin_functions[39]{};

static void unregister_functions() {
    for (unsigned index = 39; index;)
        unregister_function(&builtin_functions[--index]);
}

static void register_functions() {
    using namespace commands;
    static const struct {
        const char *name;
        Handler handler;
    } bindings[]{{"SetHook", set_hook},
                 {"SetText", set_text},
                 {"SetUserData", set_user_data},
                 {"SetColor", set_colour},
                 {"Fade", fade},
                 {"Create", create},
                 {"SetInstanceName", set_instance_name},
                 {"SetPos", set_position},
                 {"SetSize", set_size},
                 {"Move", move},
                 {"Grow", grow},
                 {"MoveGrow", move_grow},
                 {"SetWidth", set_width},
                 {"SetHeight", set_height},
                 {"SetLeft", set_left},
                 {"SetTop", set_top},
                 {"SetImage", set_image},
                 {"SetFPS", set_fps},
                 {"SetTransform", set_transform},
                 {"Transform", transform},
                 {"Hide", hide},
                 {"Show", show},
                 {"Enable", enable},
                 {"Disable", disable},
                 {"Focus", focus},
                 {"SetMaxChar", set_max_characters},
                 {"TargetSibling", target_sibling},
                 {"TargetParent", target_parent},
                 {"TriggerScript", trigger_script},
                 {"If", conditional},
                 {"EndScript", end_script},
                 {"Goto", go_to},
                 {"End", end},
                 {"Wait", wait},
                 {"Restart", restart},
                 {"Set", set},
                 {"UnSet", unset},
                 {"SetGlobal", set_global_value},
                 {"UnSetGlobal", unset_global}};
    static_assert(std::size(bindings) == 39);
    for (unsigned index = 0; index < std::size(bindings); ++index)
        register_function(&builtin_functions[index], bindings[index].name, bindings[index].handler);
    std::atexit(unregister_functions);
}
} // namespace nfs::gui::widgets::script
NFS_KERNEL_INITIALIZER(startup_register_functions, ".CRT$XCU_NFS_00690720",
                       nfs::gui::widgets::script::register_functions);
