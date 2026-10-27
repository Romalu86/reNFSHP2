#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdlib>
#include <cwchar>
namespace nfs::gui {

const void *resolve_string(Dword reference) {
    if (!string_resolver)
        return nullptr;
    using Resolve = const void *(__thiscall *)(Object *, Dword);
    return reinterpret_cast<const Resolve *>(string_resolver->methods)[1](string_resolver,
                                                                          reference);
}

int resolve_length(Dword reference) {
    if (!string_resolver)
        return 0;
    using Resolve = int(__thiscall *)(Object *, Dword);
    return reinterpret_cast<const Resolve *>(string_resolver->methods)[4](string_resolver,
                                                                          reference);
}

Dword resolve_wide(Dword reference) {
    if (!string_resolver)
        return 0;
    using Resolve = Dword(__thiscall *)(Object *, Dword);
    return reinterpret_cast<const Resolve *>(string_resolver->methods)[2](string_resolver,
                                                                          reference);
}

String *string_reference(String *text, Dword reference) {
    string_clear(text);
    if (reference && resolve_string(reference)) {
        if (text->data) {
            kernel::memory_free(text->data);
            text->data = nullptr;
        }
        text->length = 0;
        text->wide = 0;
        text->reference = reference;
        text->length = resolve_length(reference);
        text->wide = resolve_wide(reference);
    }
    return text;
}

String *string_construct_reference(String *text, Dword reference) {
    string_construct(text);
    // Native construction state0 destroys only the completed Object base.
    kernel::lifecycle::Cleanup base([&] { text->methods = base_methods; });
    string_reference(text, reference);
    base.dismiss();
    return text;
}

String *string_construct_copy(String *text, const String *source) {
    string_construct(text);
    // Native construction state0 destroys only the completed Object base.
    kernel::lifecycle::Cleanup base([&] { text->methods = base_methods; });
    string_copy(text, source);
    base.dismiss();
    return text;
}

String *string_construct_ascii(String *text, const char *source) {
    string_construct(text);
    // Native construction state0 destroys only the completed Object base.
    kernel::lifecycle::Cleanup base([&] { text->methods = base_methods; });
    string_assign(text, source);
    base.dismiss();
    return text;
}
 
bool string_make_narrow(String *text) {
    if (text->wide) {
        const auto *source = static_cast<const wchar_t *>(string_data(text));
        if (source) {
            const int length = text->length;
            auto *memory = static_cast<char *>(kernel::memory_allocate(nullptr, length + 1, pool));
            if (std::wcstombs(memory, source, length) == static_cast<std::size_t>(-1)) {
                kernel::memory_free(memory);
                return false;
            }
            memory[length] = 0;
            string_clear(text);
            if (memory) {
                text->data = memory;
                text->length = length;
            }
        }
    }
    return true;
}
// The source character count is retained after conversion.
bool string_make_wide(String *text) {
    if (text->wide)
        return true;
    const auto *source = static_cast<const char *>(string_data(text));
    if (!source)
        return true;
    const int length = text->length;
    auto *memory = static_cast<wchar_t *>(kernel::memory_allocate(nullptr, length * 2 + 2, pool));
    if (std::mbstowcs(memory, source, length) == static_cast<std::size_t>(-1)) {
        kernel::memory_free(memory);
        return false;
    }
    memory[length] = 0;
    string_clear(text);
    if (memory) {
        text->data = memory;
        text->length = length;
        text->wide = 1;
    }
    return true;
}

String *string_assign_wide(String *text, const wchar_t *source) {
    if (source == string_data(text))
        return text;
    if (source) {
        const int length = static_cast<int>(std::wcslen(source));
        text->length = length;
        if (length) {
            text->data = reallocate(text->data, "GUIStr", length * 2 + 2, 0);
            if (text->data) {
                text->wide = 1;
                text->reference = 0;
                kernel::memory_copy(text->data, source, length * 2);
                static_cast<wchar_t *>(text->data)[length] = 0;
                return text;
            }
        }
    }
    string_clear(text);
    return text;
}

void string_append_reference(String *text, Dword reference) {
    String tail;
    string_construct(&tail);
    try {
        string_reference(&tail, reference);
        string_append(text, &tail);
    } catch (...) {
        string_destruct(&tail);
        throw;
    }
    string_destruct(&tail);
}

String *copy_member_string(const void *object, String *result) {
    return string_construct_copy(
        result, reinterpret_cast<const String *>(static_cast<const char *>(object) + 4));
}
} // namespace nfs::gui
