#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
namespace nfs::gui {
namespace {
void reset_to_shared(String *text) {
    string_copy(text, &lifecycle::construction::unresolved::shared_string);
}
} // namespace
// Real CRT formatting is retained; complete legacy locale,
// %n policy, error-handler behavior and original unwind ABI remain open.
int string_format(String *text, const String *pattern, ...) {
    if (!string_data(pattern)) {
        reset_to_shared(text);
        return 0;
    }
    va_list arguments;
    va_start(arguments, pattern);
    int estimated = 0, written = 0;
    void *memory = nullptr;
    bool wide = pattern->wide != 0;
    if (!wide) {
        auto *format = static_cast<const char *>(string_data(pattern));
        if (!format) {
            wide = pattern->wide != 0;
            if (!wide) {
                va_end(arguments);
                return 0;
            }
        } else if (*format &&
            (estimated = estimate_ascii(format, reinterpret_cast<const Dword *>(arguments))) > 0) {
            memory = kernel::memory_allocate(nullptr, estimated + 1, pool | 256);
            if (memory)
                written = std::vsprintf(static_cast<char *>(memory), format, arguments);
        }
    }
    if (wide) {
        auto *format = static_cast<const wchar_t *>(string_data(pattern));
        if (!format) {
            va_end(arguments);
            return 0;
        }
        if (*format &&
            (estimated = estimate_wide(format, reinterpret_cast<const Dword *>(arguments))) > 0) {
            memory = kernel::memory_allocate(nullptr, estimated * 2 + 2, pool | 256);
            if (memory)
                written = __vswprintf_l(static_cast<wchar_t *>(memory), format, nullptr, arguments);
        }
    }
    va_end(arguments);
    if (memory && written > 0) {
        if (text->data) {
            kernel::memory_free(text->data);
            text->data = nullptr;
        }
        text->reference = 0;
        text->wide = wide ? 1 : 0;
        text->length = written;
        text->data = memory;
        return written;
    }
    if (memory)
        kernel::memory_free(memory);
    reset_to_shared(text);
    return 0;
}

int string_format_ascii(String *text, const char *pattern, ...) {
    if (pattern && *pattern) {
        va_list arguments;
        va_start(arguments, pattern);
        const int estimated = estimate_ascii(pattern, reinterpret_cast<const Dword *>(arguments));
        if (estimated > 0)
            if (void *memory = kernel::memory_allocate(nullptr, estimated + 1, pool | 256)) {
                const int written = std::vsprintf(static_cast<char *>(memory), pattern, arguments);
                va_end(arguments);
                if (written > 0) {
                    if (text->data) {
                        kernel::memory_free(text->data);
                        text->data = nullptr;
                    }
                    text->reference = 0;
                    text->wide = 0;
                    text->length = written;
                    text->data = memory;
                    return written;
                }
                kernel::memory_free(memory);
                reset_to_shared(text);
                return 0;
            }
        va_end(arguments);
    }
    reset_to_shared(text);
    return 0;
}
} // namespace nfs::gui
