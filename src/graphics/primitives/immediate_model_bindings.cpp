#include "nfs/graphics/primitives/immediate_draw.hpp"
#include <cstring>
namespace nfs::graphics::immediate {

int append_dynamic_primitive(Dword *model, Dword *primitive) {
    const int previous_capacity = int(model[17]);
    if (previous_capacity <= int(model[18])) {
        model[17] = previous_capacity < 16 ? 16 : Dword(previous_capacity) * 2;
        auto *primitives =
            static_cast<Dword *>(eagl::allocator(model[17] * 4, "Dynamic Model Geoprim list"));
        for (int index = 0; index < int(model[18]); ++index)
            primitives[index] = reinterpret_cast<Dword *>(model[19])[index];
        if (model[19])
            eagl::deallocator(reinterpret_cast<void *>(model[19]), Dword(previous_capacity) * 4);
        model[19] = reinterpret_cast<Dword>(primitives);
        auto *arrays =
            static_cast<Dword *>(eagl::allocator(model[17] * 4, "Dynamic Model Draw Array list"));
        std::memset(arrays, 0, model[17] * 4);
        for (int index = 0; index < int(model[18]); ++index)
            arrays[index] = reinterpret_cast<Dword *>(model[20])[index];
        if (model[20])
            eagl::deallocator(reinterpret_cast<void *>(model[20]), Dword(previous_capacity) * 4);
        model[20] = reinterpret_cast<Dword>(arrays);
    }
    reinterpret_cast<Dword *>(model[19])[model[18]] = reinterpret_cast<Dword>(primitive);
    const int result = int(model[18]);
    ++model[18];
    return result;
}
// Only AL is consumed by callers.
std::uint8_t bind_builder_primitive(Dword *builder, Dword *primitive) {
    builder[2] = reinterpret_cast<Dword>(primitive);
    append_dynamic_primitive(reinterpret_cast<Dword *>(builder[1]), primitive);
    builder[4] = 0;
    primitive = reinterpret_cast<Dword *>(builder[2]);
    while (reinterpret_cast<Dword *>(reinterpret_cast<Dword *>(primitive[0])[5])[builder[4]])
        ++builder[4];
    auto *values = static_cast<Dword *>(eagl::allocator(builder[4] * 8, "mUserVarData"));
    builder[3] = reinterpret_cast<Dword>(values);
    if (!values)
        return 0;
    for (int index = 0; index < int(builder[4]); ++index)
    {
        reinterpret_cast<Dword *>(builder[3])[index * 2] = 0;
        reinterpret_cast<Dword *>(builder[3])[index * 2 + 1] = 0;
    }
    return 1;
}

std::uint8_t builder_topology(Dword *builder, Dword topology) {
    switch (topology) {
    case 3:
        builder[12] = 1;
        break;
    case 5:
        builder[12] = 2;
        break;
    case ~Dword(0):
    case 1:
    case 2:
    case 4:
        builder[12] = 0;
        break;
    default:
        builder[12] = ~Dword(0);
        break;
    }
    if (builder[12] == ~Dword(0))
        return 0;
    builder[0] = topology;
    builder[9] = 16384 - builder[12];
    builder[8] = builder[9] + builder[12];
    builder[10] = builder[8];
    return 1;
}

int find_builder_parameter(Dword *builder, const char *name) {
    if (int(builder[4]) < 1)
        return -1;
    auto *primitive = reinterpret_cast<Dword *>(builder[2]);
    auto *definition = reinterpret_cast<Dword *>(primitive[0]);
    auto **names = reinterpret_cast<const char **>(definition[5]);
    for (int index = 0; index < int(builder[4]); ++index)
        if (!std::strcmp(name, names[index]))
            return index;
    return -1;
}

bool select_builder_parameters(Dword *builder, const char *name) {
    builder[11] = Dword(find_builder_parameter(builder, name));
    return builder[11] != ~Dword(0);
}
// Callers discard the partly defined EAX result.
void bind_builder_parameter_index(Dword *builder, int index, void *data, int stride, int count) {
    if (count != -1)
        reinterpret_cast<std::uint8_t *>(builder)[53] = 1;
    if (index == -1)
        return;
    auto *values = reinterpret_cast<Dword *>(builder[3]);
    values[index * 2] = reinterpret_cast<Dword>(data);
    reinterpret_cast<Dword *>(builder[3])[index * 2 + 1] = Dword(stride);
    auto *primitive = reinterpret_cast<Dword *>(builder[2]);
    if (!stride) {
        primitive[index * 2 + 2] = reinterpret_cast<Dword *>(builder[3])[index * 2];
        primitive[index * 2 + 1] = 1;
    } else if (reinterpret_cast<std::uint8_t *>(builder)[53]) {
        primitive[index * 2 + 1] = Dword(count);
    }
}
// The fourth stack argument is ignored by this wrapper.
void bind_builder_parameter(Dword *builder, const char *name, void *data, int stride, int) {
    bind_builder_parameter_index(builder, find_builder_parameter(builder, name), data, stride, -1);
}

std::uint8_t begin_model_builder(Dword *builder) {
    reinterpret_cast<std::uint8_t *>(builder)[24] = 1;
    reinterpret_cast<std::uint8_t *>(builder)[52] = 1;
    return 1;
}
} // namespace nfs::graphics::immediate
