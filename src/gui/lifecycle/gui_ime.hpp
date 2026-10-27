#pragma once
#include "../widgets/gui_widget_edit.hpp"
#include "../widgets/gui_widget_list.hpp"
#include "../widgets/gui_widget_hooks.hpp"
#include "../../frontend/flow/frontend_application_widgets.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include <array>
#include <windows.h>
#include <imm.h>
namespace nfs::gui::widgets::ime {
struct Edit {
    family::EditText base;
    HIMC detached_context;
    int composition_caret;
    std::uint16_t language, padding_152;
    String status;
    Dword properties;
    std::uint16_t pending_character;
    std::uint8_t composing, wide_os, disabled, padding_171[3];
    Dword candidate_id;
};
struct CandidateList {
    family::ListBox base;
    Dword index;
    HWND window;
    Dword codepage;
    std::uint8_t wide, state, padding_17e[2];
    Dword capacity;
    CANDIDATELIST *buffer;
};
struct DebugScreen {
    Widget base;
    Edit *edit;
};
static_assert(sizeof(Edit) == 376 && offsetof(Edit, status) == 340 &&
              offsetof(Edit, candidate_id) == 372);
static_assert(sizeof(CandidateList) == 392 && offsetof(CandidateList, buffer) == 388);
static_assert(sizeof(DebugScreen) == 260);
extern const std::array<Method, 59> edit_methods;
extern const std::array<Method, 58> candidate_methods;
extern const std::array<Method, 14> candidate_item_methods;
extern const std::array<Method, 42> debug_methods;
extern HHOOK message_hook;
extern Dword instance_count, target_id, codepage;
extern HIMC context;
extern std::uint8_t message_flag, hook_busy, unicode_composition;
const char *__cdecl edit_class_key();
const char *__cdecl candidate_class_key();
HWND input_window();
void install_message_hook();
void remove_message_hook();
Dword route_window_message(HWND, UINT, WPARAM, LPARAM);
void open_context(Edit *);
void close_context(Edit *);
bool context_open(Edit *);
void detach_context(Edit *);
void enable_context(Edit *);
std::uint8_t context_disabled(Edit *);
void select_layout(Edit *, HKL);
std::uint16_t decode_character(Edit *, std::uint16_t);
Dword composition_string(Edit *, HIMC, Dword, wchar_t *, Dword);
int compact_attributes(Edit *, const unsigned char *, std::uint8_t *);
std::uint16_t first_composition_character(Edit *);
void conversion_status(Edit *);
void notify_context(Edit *, Dword, Dword);
void toggle_settings(Edit *);
void draw_debug_atlas(Edit *);
void refresh_candidates(Edit *);
void configure_candidates(CandidateList *, HWND, Dword, std::uint8_t);
void refresh_candidate_buffer(CandidateList *);
void __fastcall begin_composition(Edit *, void *);
void __fastcall end_composition(Edit *, void *);
void __fastcall update_composition(Edit *, void *, std::uint16_t, Dword);
Dword __fastcall filter_character(Edit *, void *, Dword, Dword);
void __fastcall parent_enter(Edit *, void *, Widget *);
void __fastcall parent_leave(Edit *, void *, Dword);
Dword __fastcall character(Edit *, void *, Dword, Dword);
void __fastcall edit_operation(Edit *, void *, Dword);
Dword __fastcall navigate(Edit *, void *, Dword, Dword, Dword);
Dword __fastcall event(Edit *, void *, Dword, Dword, Dword);
void __fastcall draw_caret(Edit *, void *, engine::Engine *, Dword, Dword, Dword);
void __fastcall initialize_debug(DebugScreen *, void *);
void __fastcall draw_debug(DebugScreen *, void *, Dword);
Dword __cdecl enable_callback(void *, Dword, Dword, Dword);
Dword __cdecl activate_callback(void *, Dword, Dword, Dword);
Dword __cdecl information_callback(void *, Dword, Dword, Dword);
Dword __cdecl settings_callback(void *, Dword, Dword, Dword);
Dword __cdecl candidates_callback(void *, Dword, Dword, Dword);
} // namespace nfs::gui::widgets::ime
