#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::gui {
// Source and result may be the same String.
void string_substring(const String *source, int start, int count, String *result) {
    const int length = source->length;
    if (length && start < length) {
        if (start < 0)
            start = 0;
        if (count > 0) {
            if (length <= static_cast<int>(Dword(start) + Dword(count)))
                count = length - start;
            if (count > 0) {
                const Dword wide = source->wide;
                const void *data = string_data(source);
                if (!data)
                    return;
                const int width = wide ? 2 : 1;
                void *memory = kernel::memory_allocate(nullptr, count * width + width, pool);
                if (memory) {
                    kernel::memory_copy(memory, static_cast<const char *>(data) + start * width,
                                        count * width);
                    if (wide)
                        static_cast<wchar_t *>(memory)[count] = 0;
                    else
                        static_cast<char *>(memory)[count] = 0;
                    if (result->data) {
                        kernel::memory_free(result->data);
                        result->data = nullptr;
                    }
                    result->data = memory;
                    result->length = count;
                    result->reference = 0;
                    result->wide = wide ? 1 : 0;
                    return;
                }
            }
        }
    }
    string_copy(result, &lifecycle::construction::unresolved::shared_string);
}
// Mixed-width equality uses unsigned narrow characters.
bool string_equal(const String *first, const String *second) {
    const Dword left_reference = first->reference;
    const void *left = left_reference ? resolve_string(left_reference) : first->data;
    if (!left)
        return string_data(second) == nullptr;
    const Dword right_reference = second->reference;
    const void *right = right_reference ? resolve_string(right_reference) : second->data;
    if (!right)
        return (left_reference ? resolve_string(left_reference) : first->data) == nullptr;
    if (first->length != second->length)
        return false;
    const bool left_wide = first->wide != 0, right_wide = second->wide != 0;
    right = right_reference ? resolve_string(right_reference) : second->data;
    left = left_reference ? resolve_string(left_reference) : first->data;
    Dword index = 0;
    for (;; ++index) {
        const Dword a = left_wide ? static_cast<const std::uint16_t *>(left)[index]
                                 : static_cast<const std::uint8_t *>(left)[index];
        const Dword b = right_wide ? static_cast<const std::uint16_t *>(right)[index]
                                  : static_cast<const std::uint8_t *>(right)[index];
        if (a != b)
            return false;
        if (!a)
            return true;
    }
}
} // namespace nfs::gui
