#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdio>

namespace nfs::graphics::textures::shape {


int format_index(Dword type) {
    switch (type) {
    case 122:
        return 0;
    case 121:
        return 1;
    case 100:
    case 123:
        return 2;
    case 45:
    case 126:
        return 3;
    case 41:
    case 120:
        return 4;
    case 36:
    case 127:
        return 5;
    case 42:
    case 125:
        return 6;
    case 109:
        return 7;
    case 102:
        return 8;
    case 106:
        return 9;
    case 104:
        return 10;
    case 101:
        return 11;
    default:
        return -1;
    }
}


Dword type_from_depth(int depth) {
    switch (depth) {
    case 4:
        return 121;
    case 8:
        return 123;
    case 15:
    case 555:
    case 1555:
        return 126;
    case 16:
    case 565:
        return 120;
    case 24:
    case 888:
        return 127;
    case 32:
    case 8888:
        return 125;
    case 484:
        return 104;
    case 4444:
        return 109;
    case 6666:
        return 102;
    case 2101010:
        return 106;
    default:
        return 0;
    }
}

// Alignment is relative to the source chain's first address.
int attachment_bytes(Dword *image) {
    int result = 0;
    Dword adjustment = ((reinterpret_cast<Dword>(image) + 15) & ~15u) - reinterpret_cast<Dword>(image);
    while (image) {
        Dword length = size(image);
        if (result) {
            result = is_palette(*image & 255)
                         ? (((Dword(result) - adjustment + 31) & ~15u) - 16) + adjustment
                         : ((Dword(result) - adjustment + 15) & ~15u) + adjustment;
        }
        result += length;
        int offset = static_cast<int>(*image) >> 8;
        if (!offset)
            break;
        image = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + offset);
    }
    return result;
}

// Retain untouched alignment gaps and the last copied tag.
void copy_attachments(void *destination, Dword *image) {
    Dword cursor = 0;
    Dword adjustment = ((reinterpret_cast<Dword>(image) + 15) & ~15u) - reinterpret_cast<Dword>(image);
    Dword *previous = nullptr;
    while (image) {
        Dword length = size(image);
        if (cursor) {
            cursor = is_palette(*image & 255)
                         ? (((cursor - adjustment + 31) & ~15u) - 16) + adjustment
                         : ((cursor - adjustment + 15) & ~15u) + adjustment;
        }
        auto *output = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(destination) + cursor);
        if (previous)
            *previous = (reinterpret_cast<Dword>(output) - reinterpret_cast<Dword>(previous)) * 256 ^
                        (*previous & 255);
        *output &= 255;
        if (length) {
            kernel::memory_move(output, image, length);
            if (Byte(*output) == 'i' && length > 20)
                std::sprintf(reinterpret_cast<char *>(output + 4), "EAGL%d",
                             int(static_cast<std::int16_t>(output[1])));
        }
        cursor += length;
        int offset = static_cast<int>(*image) >> 8;
        if (!offset)
            break;
        previous = output;
        image = reinterpret_cast<Dword *>(reinterpret_cast<Dword>(image) + offset);
    }
}

// Retain all five saved allocator arguments in their recorded order.
Dword *copy_shape(Dword *image) {
    int length = attachment_bytes(image);
    auto *result = static_cast<Dword *>(allocate_aligned(shape_name(image), length, 0, 16, 1024));
    copy_attachments(result, image);
    return result;
}

// This entry marks the first chunk before releasing it.
void release_shape(Dword *image) {
    *reinterpret_cast<Byte *>(image) = 0;
    release(image);
}

} // namespace nfs::graphics::textures::shape
