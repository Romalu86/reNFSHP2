#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
#include <cstdlib>
namespace nfs::graphics::tar {
#include "../loader/eagl_tar_messages.inc"
static char shape_name[11] = "shape_";
static void set_name(const char *text) {
    for (int i = 0; i < 4; ++i)
        shape_name[i + 6] = text[i];
    shape_name[10] = 0;
}

void apply_property(Dword *object, const properties::Property *property) {
    if (property->count != 1)
        return;
    const char *key = property->name;
    const char *value = property->values[0];
    if (!std::strcmp(key, "SetClampMode")) {
        Dword mode = properties::tar_api_value(value);
        object[1] = object[2] = object[3] = object[4] = mode;
    } else if (!std::strcmp(key, "SetFilterMode"))
        object[6] = properties::tar_api_value(value);
    else if (!std::strcmp(key, "SetMIPMAPLODBias")) {
        float bias = static_cast<float>(std::atof(value));
        std::memcpy(object + 7, &bias, 4);
    } else if (!std::strcmp(key, "SetMIPMAPMode"))
        object[8] = properties::tar_api_value(value);
}

void constructor(Dword *object, eagl::Loader *loader) {
    Dword saved[12];
    for (int i = 0; i < 12; ++i)
        saved[i] = object[i];
    char name[11] = "shape_";
    for (int i = 0; i < 4; ++i)
        name[6 + i] = reinterpret_cast<const char *>(object + 1)[i];
    std::uint8_t found;
    Dword image = eagl::pool_lookup(&eagl::external_symbols, name, &found);
    if (!found) {
        Dword loaded = 0;
        eagl::address(loader, "SHAPE", name, &loaded);
        image = loaded;
    }
    if (!image)
        image =
            reinterpret_cast<Dword>(fallback_image) + reinterpret_cast<Dword *>(fallback_image)[5];
    construct_shape(object, reinterpret_cast<Dword *>(image));
    shape(object);
    if (saved[7] == 0) {
        object[1] = object[2] = object[3] = object[4] = 3;
        reinterpret_cast<Dword *>(object[10])[2] = 3;
    } else if (saved[7] == 1) {
        object[1] = object[2] = object[3] = object[4] = 1;
        reinterpret_cast<Dword *>(object[10])[2] = 1;
    } else
        eagl::report(0, constructor_clamp_message, saved[7]);
    if (saved[8] == 0)
        reinterpret_cast<Dword *>(object[10])[3] = 3;
    else if (saved[8] == 1)
        reinterpret_cast<Dword *>(object[10])[3] = 1;
    else
        eagl::report(0, constructor_filter_message, saved[8]);
}

Dword *runtime_allocate(const char *text, eagl::Loader *loader, Dword *context, std::uint8_t *owned) {
    properties::Parser parser;
    properties::parse(&parser, text);
    Dword *array = nullptr;
    int count = 1;
    for (int i = 0; i < parser.count; ++i) {
        const auto *property = parser.entries + i;
        const char *key = property->name;
        if (!std::strcmp(key, "UID")) {
            *owned = 1;
            continue;
        }
        if (!std::strcmp(key, "SHAPENAME")) {
            if (!array) {
                set_name(property->values[0]);
                count = std::atoi(property->values[1]);
                std::uint8_t found;
                Dword image = eagl::pool_lookup(&eagl::external_symbols, shape_name, &found);
                if (!found) {
                    Dword loaded;
                    eagl::address(loader, "SHAPE", shape_name, &loaded);
                }
                if (!image)
                    image = reinterpret_cast<Dword>(fallback_image) +
                            reinterpret_cast<Dword *>(fallback_image)[5];
                array = static_cast<Dword *>(
                    eagl::allocator(static_cast<Dword>(count) * 48, runtime_label));
                if (array)
                    construct_shape(array, reinterpret_cast<Dword *>(image));
                shape(array);
            }
            continue;
        }
        if (!array)
            continue;
        if (properties::match_property(property, "CLUTNAME", 1)) {
            set_name(property->values[0]);
            std::uint8_t found;
            Dword palette = eagl::pool_lookup(&eagl::external_symbols, shape_name, &found);
            if (!found) {
                Dword loaded;
                eagl::address(loader, "SHAPE", shape_name, &loaded);
            } else if (palette)
                set_palette(array, reinterpret_cast<Dword *>(palette));
            continue;
        }
        if (!std::strncmp("PCEXTOBJ", key, 8)) {
            if (property->count == 1) {
                const char *value = property->values[0];
                if (!std::strcmp(key, "PCEXTOBJ_SetStage"))
                    set_stage(array + 10, properties::tar_value(value));
                else if (!std::strcmp(key, "PCEXTOBJ_SetAnisotropy"))
                    reinterpret_cast<Dword *>(array[10])[5] = std::atoi(value);
                else if (!std::strcmp(key, "PCEXTOBJ_SetClampU"))
                    reinterpret_cast<Dword *>(array[10])[2] = properties::tar_value(value);
                else if (!std::strcmp(key, "PCEXTOBJ_SetClampV"))
                    reinterpret_cast<Dword *>(array[10])[3] = properties::tar_value(value);
                else if (properties::match_property(property, "PCEXTOBJ_SetClampW", 1))
                    reinterpret_cast<Dword *>(array[10])[4] = properties::tar_value(value);
            }
        } else
            apply_property(array, property);
    }
    for (int i = 1; i < count; ++i)
        if (array + i * 12)
            copy_construct(array + i * 12, array);
    *context = count;
    properties::destroy(&parser);
    return array;
}
} // namespace nfs::graphics::tar
