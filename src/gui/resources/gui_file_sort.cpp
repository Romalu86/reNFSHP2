#include "gui_file_dialog.hpp"
namespace nfs::gui {
// The length position participates as a zero character.
int string_find_last_character(const String *text, Dword character) {
    const int length = text->length;
    for (int index = length; index >= 0; --index) {
        Dword value = 0;
        if (index < length) {
            if (const void *data = string_data(text))
                value = text->wide ? static_cast<const std::uint16_t *>(data)[index]
                                   : Dword(static_cast<const signed char *>(data)[index]);
        }
        if (value == character)
            return index;
    }
    return -1;
}
namespace {
int compare_string_values(const String *first, const String *second, bool signed_narrow) {
    const void *left = string_data(first);
    if (!left)
        return string_data(second) ? -1 : 0;
    const void *right = string_data(second);
    if (!right)
        return 1;
    for (Dword index = 0;; ++index) {
        const int a = first->wide     ? static_cast<const std::uint16_t *>(left)[index]
                      : signed_narrow ? static_cast<const signed char *>(left)[index]
                                      : static_cast<const unsigned char *>(left)[index];
        const int b = second->wide    ? static_cast<const std::uint16_t *>(right)[index]
                      : signed_narrow ? static_cast<const signed char *>(right)[index]
                                      : static_cast<const unsigned char *>(right)[index];
        if (a != b)
            return a < b ? -1 : 1;
        if (!a)
            return 0;
    }
}
} // namespace
// Inline narrow-character comparisons use signed operands.
bool string_greater_signed(const String *first, const String *second) {
    return compare_string_values(first, second, true) > 0;
}

bool string_less_signed(const String *first, const String *second) {
    return compare_string_values(first, second, true) < 0;
}
} // namespace nfs::gui
namespace nfs::gui::widgets::family {
using application::pending::LocalFrontendString;

int __cdecl compare_file_names(const void *first, const void *second, Dword) {
    if (!first)
        return second ? 1 : 0;
    if (!second)
        return -1;
    const auto *left = static_cast<const ListItem *>(first);
    const auto *right = static_cast<const ListItem *>(second);
    const bool left_directory = (left->data & 16) != 0;
    const bool right_directory = (right->data & 16) != 0;
    if (left_directory != right_directory)
        return left_directory ? -1 : 1;
    LocalFrontendString a(&left->text), b(&right->text);
    engine::uppercase(&a.value);
    engine::uppercase(&b.value);
    return gui::compare_string_values(&a.value, &b.value, false);
}

int __cdecl compare_file_extensions(const void *first, const void *second, Dword) {
    if (!first)
        return second ? 1 : 0;
    if (!second)
        return -1;
    const auto *left = static_cast<const ListItem *>(first);
    const auto *right = static_cast<const ListItem *>(second);
    const bool left_directory = (left->data & 16) != 0;
    const bool right_directory = (right->data & 16) != 0;
    if (left_directory != right_directory)
        return left_directory ? -1 : 1;
    LocalFrontendString a(&left->text), b(&right->text);
    if (!string_data(&a.value))
        return 1;
    if (!string_data(&b.value))
        return -1;
    LocalFrontendString extension_a, extension_b, basename_a, basename_b;
    int dot = string_find_last_character(&a.value, '.');
    if (dot >= 0) {
        string_substring(&a.value, dot + 1, a.value.length, &extension_a.value);
        string_substring(&a.value, 0, dot, &basename_a.value);
    }
    dot = string_find_last_character(&b.value, '.');
    if (dot >= 0) {
        string_substring(&b.value, dot + 1, b.value.length, &extension_b.value);
        string_substring(&b.value, 0, dot, &basename_b.value);
    }
    engine::uppercase(&extension_a.value);
    engine::uppercase(&extension_b.value);
    engine::uppercase(&basename_a.value);
    engine::uppercase(&basename_b.value);
    if (string_less_signed(&extension_a.value, &extension_b.value))
        return -1;
    if (string_greater_signed(&extension_a.value, &extension_b.value))
        return 1;
    if (string_less_signed(&basename_a.value, &basename_b.value))
        return -1;
    if (string_greater_signed(&basename_a.value, &basename_b.value))
        return 1;
    return 0;
}
} // namespace nfs::gui::widgets::family
