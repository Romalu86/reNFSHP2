#include "../widgets/gui_widget_edit.hpp"
namespace nfs::gui {
String *clipboard_text{};
Dword clipboard_enabled = 1;

void set_clipboard_enabled(Dword enabled) {
    clipboard_enabled = enabled;
}

Dword copy_clipboard(const String *source) {
    if (!clipboard_enabled)
        return 0;
    if (!clipboard_text) {
        clipboard_text = static_cast<String *>(allocate_flags(sizeof(String), 256, nullptr));
        if (clipboard_text)
            string_construct(clipboard_text);
        if (!clipboard_text)
            return 0;
    }
    string_copy(clipboard_text, source);
    return 1;
}

Dword read_clipboard(String *destination) {
    if (!clipboard_enabled || !clipboard_text)
        return 0;
    string_copy(destination, clipboard_text);
    return 1;
}

Dword clear_clipboard() {
    if (!clipboard_enabled || !clipboard_text)
        return 0;
    destroy(reinterpret_cast<Object *>(clipboard_text));
    clipboard_text = nullptr;
    return 1;
}
} // namespace nfs::gui
