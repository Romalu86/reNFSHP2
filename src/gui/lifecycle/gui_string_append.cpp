#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
namespace nfs::gui {
void string_append_wide(String *, const wchar_t *);

void string_append_ascii(String *destination, const char *tail) {
    if (!tail)
        return;
    if (destination->wide) {
        String temporary;
        string_construct(&temporary);
        try {
            string_assign(&temporary, tail);
            if (string_make_wide(&temporary))
                string_append_wide(destination, temporary.wide ? static_cast<const wchar_t *>(
                                                                     string_data(&temporary))
                                                               : nullptr);
        } catch (...) {
            string_destruct(&temporary);
            throw;
        }
        string_destruct(&temporary);
        return;
    }
    int length = static_cast<int>(std::strlen(tail));
    if (!length)
        return;
    int previous = destination->length, total = length + previous;
    auto *memory = static_cast<char *>(kernel::memory_allocate(nullptr, total + 1, pool));
    if (!memory)
        return;
    *memory = 0;
    const auto *text = static_cast<const char *>(string_data(destination));
    if (text && previous)
        std::memcpy(memory, text, previous + 1);
    std::memcpy(memory + std::strlen(memory), tail, length + 1);
    if (destination->data)
        kernel::memory_free(destination->data);
    destination->reference = 0;
    destination->wide = 0;
    destination->data = memory;
    destination->length = total;
}
} // namespace nfs::gui
