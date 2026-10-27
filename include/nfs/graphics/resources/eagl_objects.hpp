#pragma once
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::graphics {
extern HWND render_window;
Dword* construct_context(Dword*,Device*);Dword* create_context(Device*);void destroy_context(Device*,Dword*);void set_context_mode(Dword*,Dword);
Dword* create_viewport(Dword*);void destroy_viewport(Dword*,Dword*);
Dword* context_set_owner(Dword*,Dword*);Dword* context_state_construct(Dword*,Dword*);
void context_default_error();void context_default_callback();void context_default_close();
namespace viewports {
extern Dword identity_values[16];extern Dword* identity;
void set_owner(Dword*,Dword*);Dword* state_construct(Dword*,Dword*);Dword* construct(Dword*,Dword*);Dword* construct_target(Dword*,Dword*);
void destroy(Dword*);void base_destroy(Dword*);void set_data(Dword*,Dword);Dword data(const Dword*);
void parameters(const Dword*,Dword*,Dword*,Dword*,Dword*);void set_color(Dword*,Dword);Dword* matrix(Dword*);
Dword* current(Dword*);void select(Dword**,Dword*);Dword register_symbol(const char*,Dword);
}
}
