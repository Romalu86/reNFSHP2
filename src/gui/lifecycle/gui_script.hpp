#pragma once
#include "../../frontend/flow/frontend_application_widgets.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
#include "gui_registry_owners.hpp"
#include "../widgets/gui_widget_hooks.hpp"
#include "../widgets/gui_widget_resources.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <array>
namespace nfs::gui::engine::pending {
int string_integer(const String *);
long double string_number(const String *);
Dword load_text_resource(const String *, String *);
} // namespace nfs::gui::engine::pending
namespace nfs::gui::widgets::script {
struct Script;
struct Command;
using Handler = Dword(__cdecl *)(Script *, Command *, int);
struct FunctionRecord {
    Handler call;
    Dword hash;
    FunctionRecord *next;
};
struct Command {
    const Destroy *methods;
    Container arguments;
    FunctionRecord *function;
    int elapsed;
};
struct Variable {
    const Destroy *methods;
    String name, value;
};
struct Script {
    Widget base;
    Container commands, variables;
    String uid;
    Dword position, target;
    int state, elapsed;
    Dword busy, style;
    String source;
};
static_assert(sizeof(FunctionRecord) == 12 && sizeof(Command) == 28 &&
              offsetof(Command, elapsed) == 24 && sizeof(Variable) == 44);
static_assert(sizeof(Script) == 352 && offsetof(Script, position) == 308 &&
              offsetof(Script, source) == 332);
extern FunctionRecord *functions;
extern const std::array<Method, 42> script_methods;
extern const Destroy variable_methods[], command_methods[];
const char *__cdecl class_key();
inline const String *empty() {
    return &lifecycle::construction::unresolved::shared_string;
}
void trim_leading(String *, const char *);
void trim_trailing(String *, const char *);
void trim_leading_spaces(String *);
void trim_trailing_spaces(String *);
Script *find_script(const String *);
const String *find_global(const String *);
const String *find_local(Script *, const String *);
const String *resolve(Script *, const String *);
const String *argument(Command *, Dword, Script *);
void set_global(const String *, const String *);
void set_local(Script *, const String *, const String *);
void register_function(FunctionRecord *, const char *, Handler);
void link_function(FunctionRecord *);
void unregister_function(FunctionRecord *);
void parse(Script *, const String *);
void advance(Script *, int);
void start(Script *, Dword);
void pause(Script *, Dword);
void modal_driver(void *);
Command *construct_command(Command *, const String *);
void destroy_command(Command *);
Widget *target(Script *);
Script *first_child_script(Widget *);
Dword execute_command(Command *, Script *, int);
Dword __fastcall event(Script *, void *, Dword, Dword, Dword);
Dword __fastcall load(Script *, void *, void *);
Dword __fastcall save(Script *, void *, void *);
void __fastcall advance_frame(Script *, void *, int);
namespace commands {
Dword __cdecl set_hook(Script *, Command *, int);
Dword __cdecl set_text(Script *, Command *, int);
Dword __cdecl set_user_data(Script *, Command *, int);
Dword __cdecl set_colour(Script *, Command *, int);
Dword __cdecl fade(Script *, Command *, int);
Dword __cdecl create(Script *, Command *, int);
Dword __cdecl set_instance_name(Script *, Command *, int);
Dword __cdecl set_position(Script *, Command *, int);
Dword __cdecl set_size(Script *, Command *, int);
Dword __cdecl move(Script *, Command *, int);
Dword __cdecl grow(Script *, Command *, int);
Dword __cdecl move_grow(Script *, Command *, int);
Dword __cdecl set_width(Script *, Command *, int);
Dword __cdecl set_height(Script *, Command *, int);
Dword __cdecl set_left(Script *, Command *, int);
Dword __cdecl set_top(Script *, Command *, int);
Dword __cdecl set_image(Script *, Command *, int);
Dword __cdecl set_fps(Script *, Command *, int);
Dword __cdecl set_transform(Script *, Command *, int);
Dword __cdecl transform(Script *, Command *, int);
Dword __cdecl hide(Script *, Command *, int);
Dword __cdecl show(Script *, Command *, int);
Dword __cdecl enable(Script *, Command *, int);
Dword __cdecl disable(Script *, Command *, int);
Dword __cdecl focus(Script *, Command *, int);
Dword __cdecl set_max_characters(Script *, Command *, int);
Dword __cdecl target_sibling(Script *, Command *, int);
Dword __cdecl target_parent(Script *, Command *, int);
Dword __cdecl trigger_script(Script *, Command *, int);
Dword __cdecl conditional(Script *, Command *, int);
Dword __cdecl end_script(Script *, Command *, int);
Dword __cdecl go_to(Script *, Command *, int);
Dword __cdecl end(Script *, Command *, int);
Dword __cdecl wait(Script *, Command *, int);
Dword __cdecl restart(Script *, Command *, int);
Dword __cdecl set(Script *, Command *, int);
Dword __cdecl unset(Script *, Command *, int);
Dword __cdecl set_global_value(Script *, Command *, int);
Dword __cdecl unset_global(Script *, Command *, int);
} // namespace commands
} // namespace nfs::gui::widgets::script
