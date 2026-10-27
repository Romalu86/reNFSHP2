#pragma once
#include "../widgets/gui_widget_list.hpp"
#include "../widgets/gui_widget_list_sort.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
#include <array>
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::gui {
int string_find_last_character(const String *, Dword);
bool string_less_signed(const String *, const String *);
bool string_greater_signed(const String *, const String *);
} // namespace nfs::gui
namespace nfs::gui::widgets::family {
struct FileDialog {
    GeneralWidget base;
    Dword accept_id, cancel_id, mode, previous_focus;
    float timeout, elapsed;
    String filter, directory, filename;
    Dword attributes, sort_mode, list_id;
};
static_assert(sizeof(FileDialog) == 360 && offsetof(FileDialog, filter) == 288 &&
              offsetof(FileDialog, directory) == 308 && offsetof(FileDialog, filename) == 328);
extern const std::array<Method, 52> file_dialog_methods;
const char *__cdecl file_dialog_class_key();
void __fastcall initialize_file_dialog(FileDialog *, void *);
void __fastcall file_dialog_child(FileDialog *, void *, Widget *, Dword, Dword, Dword);
void __fastcall accept_file_dialog(FileDialog *, void *);
void __fastcall refresh_file_dialog(FileDialog *, void *);
FileDialog *create_file_dialog(const String *, const String *, const String *, void *,
                               const String *, const String *, const String *, Dword, Dword);
using FileVisitor = Dword(__cdecl *)(ListBox *, Dword, const String *, Dword, Dword, Dword);
int populate_directory_list(ListBox *, Dword, const String *, const String *, std::uint8_t, Dword,
                            FileVisitor, Dword);
String *filename_stem(String *, String);
int __cdecl compare_file_names(const void *, const void *, Dword);
int __cdecl compare_file_extensions(const void *, const void *, Dword);
} // namespace nfs::gui::widgets::family
