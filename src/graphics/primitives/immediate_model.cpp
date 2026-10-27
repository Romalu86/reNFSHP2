#include "nfs/graphics/primitives/immediate_draw.hpp"
#include <cstring>
namespace nfs::graphics::immediate {

Dword *dynamic_model_construct(Dword *object) {
    for (unsigned i = 16; i != 21; ++i)
        object[i] = 0;
    for (unsigned i = 0; i != 16; ++i)
        object[i] = i % 5 == 0 ? 1065353216 : 0;
    return object;
}

void dynamic_model_destroy(Dword *object) {
    if (object[76 / 4])
        eagl::deallocator(reinterpret_cast<void *>(object[76 / 4]), object[68 / 4] * 4);
    object[76 / 4] = 0;
    if (object[80 / 4]) {
        for (int i = 0; i < static_cast<int>(object[72 / 4]); ++i) {
            auto *item = reinterpret_cast<Dword *>(reinterpret_cast<Dword *>(object[80 / 4])[i]);
            if (item) {
                model_builder_destroy(item);
                eagl::deallocator(item, 76);
            }
            reinterpret_cast<Dword *>(object[80 / 4])[i] = 0;
        }
        eagl::deallocator(reinterpret_cast<Dword *>(object[80 / 4]), object[68 / 4] * 4);
    }
    object[80 / 4] = 0;
}

void dynamic_model_matrix(Dword *object, const Dword *matrix) {
    for (unsigned i = 0; i != 16; ++i)
        object[i] = matrix[i];
}

Dword *model_builder_construct(Dword *object) {
    object[0] = 1;
    for (unsigned i = 1; i <= 5; ++i)
        object[i] = 0;
    reinterpret_cast<std::uint8_t *>(object)[24] = 0;
    for (unsigned i = 7; i <= 10; ++i)
        object[i] = 0;
    object[11] = 4294967295u;
    object[12] = 0;
    reinterpret_cast<std::uint8_t *>(object)[52] = 1;
    reinterpret_cast<std::uint8_t *>(object)[53] = 0;
    object[14] = 0;
    auto *model = static_cast<Dword *>(eagl::allocator(88, "EAGL::DynamicModel new"));
    object[1] = reinterpret_cast<Dword>(model ? dynamic_model_construct(model) : nullptr);
    object[18] = object[17] = object[16] = object[15] = 0;
    return object;
}

void model_builder_destroy(Dword *object) {
    if (reinterpret_cast<std::uint8_t *>(object)[24])
        stop_model_builder(object);
    if (object[3])
        eagl::deallocator(reinterpret_cast<void *>(object[3]), object[4] * 8);
    auto *model = reinterpret_cast<Dword *>(object[1]);
    if (model) {
        dynamic_model_destroy(model);
        eagl::deallocator(model, 88);
    }
    object[1] = object[3] = 0;
    reinterpret_cast<std::uint8_t *>(object)[24] = 0;
}

std::uint8_t stop_model_builder(Dword *object) {
    reinterpret_cast<std::uint8_t *>(object)[24] = 0;
    return 1;
}

std::uint8_t model_builder_value(Dword *object, Dword value) {
    object[14] = value;
    return 1;
}

std::uint8_t model_builder_matrix(Dword *object, const Dword *matrix) {
    dynamic_model_matrix(reinterpret_cast<Dword *>(object[1]), matrix);
    return 1;
}

void model_builder_bindings(Dword *object, Dword) {
    int count = static_cast<int>(object[7]);
    if (static_cast<int>(object[8]) < count)
        count = object[8];
    auto *bindings = reinterpret_cast<Dword *>(object[2]);
    for (int i = 0; i < static_cast<int>(object[4]); ++i) {
        auto dynamic = reinterpret_cast<std::uint8_t *>(object)[53];
        bindings[i * 2 + 2] = reinterpret_cast<const Dword *>(object[3])[i * 2];
        if (!dynamic)
            bindings[i * 2 + 1] = count;
    }
    if (object[11] != 4294967295u) {
        object[16] = object[15] = count;
        object[17] = 0;
        object[18] = !reinterpret_cast<std::uint8_t *>(object)[52];
        bindings[object[11] * 2 + 2] = reinterpret_cast<Dword>(object + 15);
        bindings[object[11] * 2 + 1] = 1;
    }
}
} // namespace nfs::graphics::immediate
