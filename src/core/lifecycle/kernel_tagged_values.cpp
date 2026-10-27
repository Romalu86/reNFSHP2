#include "kernel_values.hpp"
namespace nfs::game::runtime::values {

static void *__fastcall delete_value(TaggedValue *value, void *, Dword flags) {
    destroy_value(value);
    if (flags & 1)
        gui::release(value);
    return value;
}

const gui::Destroy tagged_methods[] = {reinterpret_cast<gui::Destroy>(delete_value)};

void clear_value(TaggedValue *value) {
    value->type = 0;
    value->payload[0] = 0;
    value->payload[1] = 0;
    value->payload[2] = 0;
    value->payload[3] = 0;
}

void reset_value(TaggedValue *value) {
    const Dword type = value->type;
    if (type == 14 && value->payload[0]) {
        kernel::memory_free(reinterpret_cast<void *>(value->payload[0]));
        value->payload[0] = 0;
    }
    if (type == 13 && value->payload[0]) {
        kernel::memory_free(reinterpret_cast<void *>(value->payload[0]));
        value->payload[0] = 0;
    }
    clear_value(value);
}

void destroy_value(TaggedValue *value) {
    const Dword type = value->type;
    value->methods = tagged_methods;
    if (type == 14 && value->payload[0]) {
        kernel::memory_free(reinterpret_cast<void *>(value->payload[0]));
        value->payload[0] = 0;
    }
    if (type == 13 && value->payload[0]) {
        kernel::memory_free(reinterpret_cast<void *>(value->payload[0]));
        value->payload[0] = 0;
    }
    clear_value(value);
    value->methods = gui::base_methods;
}

bool equal_values(const TaggedValue *first, const TaggedValue *second) {
    const Dword type = first->type;
    if (second->type != type)
        return false;
    if (type == 14) {
        auto *left = reinterpret_cast<const unsigned char *>(first->payload[0]);
        auto *right = reinterpret_cast<const unsigned char *>(second->payload[0]);
        if (left && right) {
            const int right_size = kernel::memory_size(right);
            const int left_size = kernel::memory_size(left);
            if (left_size == right_size) {
                Dword remaining = kernel::memory_size(left);
                while (remaining) {
                    --remaining;
                    if (*left++ != *right++)
                        return false;
                }
                return true;
            }
        }
        return left == reinterpret_cast<const unsigned char *>(second->payload[0]);
    }
    if (type != 13) {
        for (unsigned index = 0; index < 4; ++index)
            if (second->payload[index] != first->payload[index])
                return false;
        return true;
    }
    auto *left = reinterpret_cast<const unsigned char *>(first->payload[0]);
    auto *right = reinterpret_cast<const unsigned char *>(second->payload[0]);
    if (!left || !right)
        return left == reinterpret_cast<const unsigned char *>(second->payload[0]);
    for (;;) {
        unsigned char character = left[0];
        if (character != right[0])
            return false;
        if (!character)
            return true;
        character = left[1];
        if (character != right[1])
            return false;
        left += 2;
        right += 2;
        if (!character)
            return true;
    }
}
} // namespace nfs::game::runtime::values
