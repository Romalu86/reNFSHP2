#include "../../frontend/network/frontend_network_pages.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include <cwctype>
namespace nfs::gui::widgets::family {

ListItem *construct_ascii_list_item(ListItem *item, const char *text) {
    return application::frontend::construct_ascii_item(item, text, 0);
}

int __cdecl compare_list_text(const void *first, const void *second, Dword) {
    if (!first)
        return second != nullptr;
    if (!second)
        return -1;
    const String *left = &static_cast<const ListItem *>(first)->text;
    const String *right = &static_cast<const ListItem *>(second)->text;
    if (!string_data(left) && !string_data(right))
        return 0;
    if (!engine::pending::character_at(left, 0))
        return 1;
    if (!engine::pending::character_at(right, 0))
        return -1;
    const void *right_data = string_data(right), *left_data = string_data(left);
    auto folded = [](const void *data, Dword wide, unsigned index) -> int {
        if (wide)
            return std::uint16_t(std::towlower(static_cast<const std::uint16_t *>(data)[index]));
        const int code = static_cast<const signed char *>(data)[index];
        return code >= 'A' && code <= 'Z' ? code + 32 : code;
    };
    for (unsigned index = 0;; ++index) {
        const int a = folded(left_data, left->wide, index);
        const int b = folded(right_data, right->wide, index);
        if (a != b) {
            const int folded_right = folded(right_data, right->wide, index);
            const int folded_left = folded(left_data, left->wide, index);
            return folded_right < folded_left ? 1 : -1;
        }
        if (!a)
            return 0;
    }
}
} // namespace nfs::gui::widgets::family
