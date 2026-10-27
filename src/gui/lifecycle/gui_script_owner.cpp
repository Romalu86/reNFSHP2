#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "gui_script.hpp"
#include "gui_configuration_values.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
namespace nfs::gui::widgets::script {
namespace {
const engine::pending::FlagDefinition style_flags[]{{"SCS_TRIGGERONLOAD", 0, 1},
                                                    {"SCS_TRIGGERONEXIT", 0, 4},
                                                    {"SCS_TRIGGERMODAL", 0, 8},
                                                    {nullptr, 0, 0}};
}

Dword __fastcall load(Script *script, void *, void *configuration) {
    using namespace engine::pending;
    read_string(configuration, "GScript.mUID", &script->uid, empty());
    read_string(configuration, "GScript.mScript", &script->source, empty());
    read_flags(configuration, "GScript.mScriptStyle", &script->style, style_flags, 0);
    if (script->source.length) {
        application::pending::LocalFrontendString text;
        if (load_text_resource(&script->source, &text.value))
            parse(script, &text.value);
    }
    return pending::load_configuration(&script->base, nullptr, configuration);
}

Dword __fastcall save(Script *script, void *, void *configuration) {
    using namespace engine::pending;
    write_text(configuration, "GScript.mUID", string_ascii(&script->uid), nullptr);
    write_ascii(configuration, "GScript.mScript", string_ascii(&script->source), "FILE", 0,
                nullptr);
    write_flags(configuration, "GScript.mScriptStyle", script->style, style_flags, 0);
    return pending::save_configuration(&script->base, nullptr, configuration);
}

static Script *construct_script(Script *script, void *parent) {
    construct_base(script, parent);
    int constructed = 0;
    try {
        container_construct(&script->commands);
        constructed = 1;
        container_construct(&script->variables);
        constructed = 2;
        string_construct(&script->uid);
        constructed = 3;
        string_construct(&script->source);
        constructed = 4;
        script->target = 0;
        script->base.methods = script_methods.data();
        script->base.flags = (script->base.flags & ~Dword(4)) | 3;
        container_append(&script_instances, script, nullptr);
        script->position = 0;
        script->state = script->elapsed = 0;
        script->style = script->busy = 0;
        if (parent)
            script->target = static_cast<Widget *>(parent)->id;
    } catch (...) {
        try {
            if (constructed >= 4)
                string_destruct(&script->source);
            if (constructed >= 3)
                string_destruct(&script->uid);
            if (constructed >= 2)
                container_destruct(&script->variables);
            if (constructed >= 1)
                container_destruct(&script->commands);
            destroy_base(&script->base);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    return script;
}

static void destroy_script(Script *script) {
    script->base.methods = script_methods.data();
    int cleanup_state = 4;
    try {
        container_detach(&script_instances, script);
        string_destruct(&script->source);
        string_destruct(&script->uid);
        cleanup_state = 5;
        container_destruct(&script->variables);
        cleanup_state = 6;
        container_destruct(&script->commands);
        cleanup_state = -1;
        destroy_base(&script->base);
    } catch (...) {
        try {
            if (cleanup_state == 4) {
                string_destruct(&script->source);
                string_destruct(&script->uid);
                container_destruct(&script->variables);
            } else if (cleanup_state == 5)
                script->variables.methods = gui::base_methods;
            if (cleanup_state == 4 || cleanup_state == 5)
                container_destruct(&script->commands);
            else if (cleanup_state == 6)
                script->commands.methods = gui::base_methods;
            if (cleanup_state >= 0)
                destroy_base(&script->base);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
}

static Script *__fastcall delete_script(Script *script, void *, Dword flags) {
    destroy_script(script);
    if (flags & 1)
        release(script);
    return script;
}

static const char *__fastcall name(Script *, void *) {
    return "GScript";
}

const char *__cdecl class_key() {
    return "GScript";
}

static Dword __fastcall is_type(Script *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(class_key) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall is_name(Script *, void *, const char *name) {
    return !std::strncmp(name, "GScript", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 42> script_methods = {
    reinterpret_cast<Method>(::nfs::gui::widgets::script::delete_script), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::name), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::is_type), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::request_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::script::advance_frame), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_32), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_36), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

static void *create_script(void *parent) {
    auto *script = static_cast<Script *>(allocate_flags(sizeof(Script), 0, "GScript"));
    if (!script)
        return nullptr;
    try {
        return construct_script(script, parent);
    } catch (...) {
        release(script);
        throw;
    }
}
static ClassRecord script_class;

static void unregister_script() {
    unregister_class(&script_class);
}

static void register_script() {
    register_class(&script_class, "GScript", create_script, nullptr);
    std::atexit(unregister_script);
}
} // namespace nfs::gui::widgets::script
NFS_KERNEL_INITIALIZER(startup_register_script, ".CRT$XCU_NFS_00690714",
                       nfs::gui::widgets::script::register_script);
NFS_KERNEL_INITIALIZER(startup_initialize_global_script_variables, ".CRT$XCU_NFS_00690718",
                       nfs::gui::initialize_global_script_variables);
NFS_KERNEL_INITIALIZER(startup_initialize_script_instances, ".CRT$XCU_NFS_0069071C",
                       nfs::gui::initialize_script_instances);
