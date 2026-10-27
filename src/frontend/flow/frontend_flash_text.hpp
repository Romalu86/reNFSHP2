#pragma once
#include "../screens/frontend_menu_controls.hpp"
namespace nfs::application::frontend {
struct FlashText {
    GeneralWidget base;
    Dword text_style, password_character;
    int margin, line_spacing;
    Dword timer;
    float progress;
    std::uint8_t active, padding[3];
    float speed, grow_x, grow_y;
    Dword style, destination_colour;
};
static_assert(sizeof(FlashText) == 312 && offsetof(FlashText, timer) == 280);
extern const std::array<Method, 49> flash_text_methods;
const char *__cdecl flash_text_class_key();
FlashText *construct_flash_text(FlashText *, void *);
void destroy_flash_text(FlashText *);
Dword __fastcall load_flash_text(FlashText *, void *, void *);
Dword __fastcall save_flash_text(FlashText *, void *, void *);
void __fastcall begin_flash_text(FlashText *, void *);
void __fastcall flash_text_timer(FlashText *, void *, Dword);
void __fastcall draw_flash_text(FlashText *, void *, gui::engine::Engine *);
void __fastcall initialize_flash_text(FlashText *, void *);
Dword __fastcall flash_text_event(FlashText *, void *, Dword, Dword, Dword);
} // namespace nfs::application::frontend
