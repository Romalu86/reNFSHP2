#include "../resources/gui_file_dialog.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "nfs/resources/files/directory_search.hpp"
#include <cstring>
namespace nfs::gui::widgets::family {
using application::pending::LocalFrontendString;
using game::runtime::invoke;
namespace {
struct LocalFileItem {
    ListItem value;
    LocalFileItem() {
        profiles::construct_profile_list_item(&value);
    }
    explicit LocalFileItem(const String *text) {
        construct_text_list_item(&value, text);
    }
    ~LocalFileItem() {
        string_destruct(&value.tooltip);
        string_destruct(&value.text);
        value.methods = reinterpret_cast<const Method *>(gui::base_methods);
    }
};
} // namespace

String *filename_stem(String *out, String name) {
    try {
        const int dot = string_find_last_character(&name, '.');
        if (dot >= 0)
            string_substring(&name, 0, dot, &name);
        string_construct_copy(out, &name);
    } catch (...) {
        string_destruct(&name);
        throw;
    }
    string_destruct(&name);
    return out;
}

int populate_directory_list(ListBox *list, Dword column, const String *directory,
                            const String *filter, std::uint8_t options, Dword attributes,
                            FileVisitor visitor, Dword argument) {
    LocalFrontendString path(directory);
    files::DirectorySearch search{};
    search.attributes = attributes;
    char previous_directory[260]{};
    files::get_directory(previous_directory);
    if (path.value.length)
        files::set_directory(string_ascii(&path.value));
    files::get_directory(search.name);
    string_assign(&path.value, search.name);
    Dword next_row = list->items.grid.rows.count;
    LocalFileItem item;
    const char *pattern = filter->length ? string_ascii(filter) : "*.*";
    Dword matched = 0;
    if (files::find_first(pattern, &search)) {
        do {
            if (!*search.name || !std::strcmp(search.name, ".") ||
                (!std::strcmp(search.name, "..") && !(attributes & 16)) ||
                !(search.attributes & attributes))
                continue;
            LocalFrontendString name(search.name);
            ++matched;
            if (visitor) {
                if (matched == 1)
                    next_row = list->items.grid.rows.count;
                const Dword row = next_row++;
                if (!visitor(list, search.attributes, &name.value, row, column, argument))
                    break;
            } else {
                if (options & 1) {
                    LocalFrontendString argument_copy(&name.value);
                    String stem;
                    filename_stem(&stem, argument_copy.release());
                    try {
                        string_copy(&name.value, &stem);
                    } catch (...) {
                        string_destruct(&stem);
                        throw;
                    }
                    string_destruct(&stem);
                }
                {
                    LocalFileItem copy(&name.value);
                    copy_list_item(&item.value, &copy.value);
                }
                if (options & 4)
                    item.value.data = search.attributes;
                if (matched == 1)
                    next_row = list->items.grid.rows.count;
                const Dword row = next_row++;
                invoke(&list->items, 7, &item.value, row, column);
            }
        } while (files::find_next(&search));
    }
    files::find_close(&search);
    files::set_directory(previous_directory);
    return int(matched);
}
} // namespace nfs::gui::widgets::family
