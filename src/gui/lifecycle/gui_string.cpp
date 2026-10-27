#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdlib>
#include <cwctype>
namespace nfs::gui {
void string_append_wide(String *, const wchar_t *);

void string_clear(String *s) {
    if (s->data) {
        kernel::memory_free(s->data);
        s->data = nullptr;
    }
    s->reference = 0;
    s->length = 0;
    s->wide = 0;
}

void *__fastcall string_delete(String *s, void *, unsigned flags) {
    string_destruct(s);
    if (flags & 1)
        release(s);
    return s;
}
void pool_string_destruct(String *);

PoolString *__fastcall pool_string_delete(PoolString *s, void *, unsigned flags) {
    pool_string_destruct(&s->string);
    if (flags & 1)
        release(s);
    return s;
}
const Destroy string_methods[] = {reinterpret_cast<Destroy>(string_delete)};
static const Destroy pool_string_methods[] = {reinterpret_cast<Destroy>(pool_string_delete)};
// Read data before changing the dynamic type.
void pool_string_destruct(String *s) {
    void *data = s->data;
    s->methods = string_methods;
    if (data) {
        kernel::memory_free(data);
        s->data = nullptr;
    }
    s->reference = 0;
    s->length = 0;
    s->wide = 0;
    s->methods = base_methods;
}

void string_destruct(String *s) {
    s->methods = string_methods;
    kernel::lifecycle::Cleanup base([&] { s->methods = base_methods; });
    string_clear(s);
}
void string_construct(String *s) {
    s->methods = string_methods;
    s->data = nullptr;
    s->reference = 0;
    s->length = 0;
    s->wide = 0;
}

const void *string_data(const String *s) {
    return s->reference ? resolve_string(s->reference) : s->data;
}

const char *string_ascii(const String *s) {
    return s->wide ? nullptr : static_cast<const char *>(string_data(s));
}

String *string_assign(String *s, const char *value) {
    if (value == string_data(s))
        return s;
    if (value) {
        int length = static_cast<int>(std::strlen(value));
        s->length = length;
        if (length) {
            s->data = reallocate(s->data, "GUIStr", length + 1, 0);
            if (s->data) {
                s->wide = 0;
                s->reference = 0;
                kernel::memory_copy(s->data, value, length);
                static_cast<char *>(s->data)[length] = 0;
                return s;
            }
        }
    }
    string_clear(s);
    return s;
}

int string_compare_inline_ascii(const String *s, const char *value) {
    const void *data = string_data(s);
    if (!data)
        return value ? -1 : 0;
    if (!value)
        return 1;
    if (!s->wide) {
        auto *left = static_cast<const signed char *>(data);
        auto *right = reinterpret_cast<const signed char *>(value);
        while (*left == *right) {
            if (!*left)
                return 0;
            ++left;
            ++right;
        }
        return *left < *right ? -1 : 1;
    }
    auto *left = static_cast<const std::uint16_t *>(data);
    auto *right = reinterpret_cast<const signed char *>(value);
    while (int(*left) == int(*right)) {
        if (!*left)
            return 0;
        ++left;
        ++right;
    }
    return int(*left) < int(*right) ? -1 : 1;
}

static int __cdecl pool_compare(const void *item, const void *key) {
    return string_compare_inline_ascii(&static_cast<const PoolString *>(item)->string,
                                       static_cast<const char *>(key));
}
// use UNSIGNED narrow bytes, unlike the
// compiler-inlined comparisons above. These must not share a signed comparator.
int string_compare_ascii(const String *s, const char *value) {
    const void *data = string_data(s);
    if (!data)
        return value ? -1 : 0;
    if (!value)
        return 1;
    auto *right = reinterpret_cast<const unsigned char *>(value);
    Dword i = 0, left;
    for (;; ++i) {
        left = s->wide ? static_cast<const std::uint16_t *>(data)[i]
                       : static_cast<const unsigned char *>(data)[i];
        if (left != right[i])
            return left < right[i] ? -1 : 1;
        if (!left)
            return 0;
    }
}
// all data/reference and wide branches.
void string_copy(String *destination, const String *source) {
    if (destination == source)
        return;
    if (!string_data(source)) {
        string_clear(destination);
        return;
    }
    if (source->reference) {
        if (destination->data) {
            kernel::memory_free(destination->data);
            destination->data = nullptr;
        }
        destination->reference = source->reference;
        destination->length = source->length;
        destination->wide = source->wide;
        return;
    }
    if (destination->data) {
        int bytes = kernel::memory_size(source->data);
        destination->data = reallocate(destination->data, "GUIStr", bytes, 0);
        if (destination->data) {
            kernel::memory_copy(destination->data, source->data, bytes);
            destination->wide = source->wide;
            destination->length = source->length;
            destination->reference = 0;
            return;
        }
        destination->length = 0;
        destination->wide = 0;
        destination->reference = 0;
        return;
    }
    destination->reference = 0;
    destination->length = 0;
    destination->wide = 0;
    int bytes = kernel::memory_size(source->data);
    destination->data = kernel::memory_allocate(nullptr, bytes, pool);
    kernel::memory_copy(destination->data, source->data, bytes);
    destination->wide = source->wide;
    destination->length = source->length;
}

PoolString *string_acquire(const char *value) {
    if (!value)
        return nullptr;
    auto *found = static_cast<PoolString *>(container_find(pooled_strings, pool_compare, value));
    if (found && string_data(&found->string) && !string_compare_ascii(&found->string, value)) {
        ++found->references;
        return found;
    }
    auto *result = static_cast<PoolString *>(allocate_flags(24, 0, "PoolStr"));
    if (result) {
        String temporary;
        string_construct(&temporary);
        try {
            string_assign(&temporary, value);
        } catch (...) {
            temporary.methods = base_methods;
            release_flags(result, 0, "PoolStr");
            throw;
        }
        string_construct(&result->string);
        try {
            string_copy(&result->string, &temporary);
        } catch (...) {
            result->string.methods = base_methods;
            string_destruct(&temporary);
            release_flags(result, 0, "PoolStr");
            throw;
        }
        result->string.methods = pool_string_methods;
        result->references = 1;
        string_destruct(&temporary);
    }
    if (result) {
        if (!container_sorted_insert(pooled_strings, pool_compare, value, result, dispose_object))
            return result;
        destroy(reinterpret_cast<Object *>(result));
    }
    return nullptr;
}

void string_release(PoolString *value) {
    if (!value)
        return;
    auto *found = static_cast<PoolString *>(
        container_find(pooled_strings, pool_compare, string_ascii(&value->string)));
    if (!found)
        return;
    const void *first = string_data(&found->string);
    const void *second = string_data(&value->string);
    bool equal = false;
    if (!first)
        equal = second == nullptr;
    else if (!second)
        equal = string_data(&found->string) == nullptr;
    else if (found->string.length == value->string.length) {
        if (!found->string.wide && !value->string.wide)
            equal = !string_compare_ascii(&found->string, string_ascii(&value->string));
        else {
            Dword i = 0, a, b;
            do {
                a = found->string.wide ? static_cast<const std::uint16_t *>(first)[i]
                                       : static_cast<const unsigned char *>(first)[i];
                b = value->string.wide ? static_cast<const std::uint16_t *>(second)[i]
                                       : static_cast<const unsigned char *>(second)[i];
                ++i;
            } while (a == b && a);
            equal = a == b;
        }
    }
    if (equal) {
        --found->references;
        if (!found->references)
            container_remove(pooled_strings, found);
    }
}
// Narrow folding is the original default C-locale path;
// non-C old-CRT locale tables remain an explicit compatibility dependency.
bool string_equal_folded(const String *s, const char *value) {
    const void *data = string_data(s);
    if (!data)
        return !value;
    if (!value)
        return false;
    auto fold = [](int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; };
    Dword i = 0;
    for (;; ++i) {
        int left =
            s->wide ? static_cast<int>(std::towlower(static_cast<const std::uint16_t *>(data)[i]))
                    : fold(static_cast<const signed char *>(data)[i]);
        int right = fold(static_cast<signed char>(value[i]));
        if (left != right)
            return false;
        if (!left)
            return true;
    }
}

static void string_take_wide(String *s, void *memory, int length) {
    string_clear(s);
    if (memory) {
        s->data = memory;
        s->length = length;
        s->wide = 1;
    }
}

void string_append(String *text, const String *tail) {
    if (!tail->length)
        return;
    if (tail->wide && !text->wide) {
        const auto *data = static_cast<const char *>(string_data(text));
        if (data) {
            const int length = text->length;
            auto *memory =
                static_cast<wchar_t *>(kernel::memory_allocate(nullptr, length * 2 + 2, pool));
            if (std::mbstowcs(memory, data, length) == static_cast<std::size_t>(-1)) {
                release(memory);
                return;
            }
            memory[length] = 0;
            string_take_wide(text, memory, length);
        }
    }
    if (!text->wide || tail->wide) {
        const int first = text->length, second = tail->length, total = first + second;
        const void *data = string_data(tail);
        if (!data)
            return;
        const Dword wide = tail->wide ? 1 : 0;
        const int width = wide ? 2 : 1;
        void *memory = kernel::memory_allocate(nullptr, total * width + width, pool);
        if (memory) {
            if (first)
                kernel::memory_copy(memory, string_data(text), first * width);
            if (second)
                kernel::memory_copy(static_cast<char *>(memory) + first * width, data,
                                    second * width);
            std::memset(static_cast<char *>(memory) + total * width, 0, width);
        }
        string_clear(text);
        if (!memory)
            return;
        text->length = total;
        text->wide = wide;
        text->data = memory;
    } else {
        String temporary;
        string_construct(&temporary);
        try {
            string_copy(&temporary, tail);
            if (string_make_wide(&temporary))
                string_append_wide(text, temporary.wide
                                             ? static_cast<const wchar_t *>(string_data(&temporary))
                                             : nullptr);
        } catch (...) {
            string_destruct(&temporary);
            throw;
        }
        string_destruct(&temporary);
    }
}
} // namespace nfs::gui
