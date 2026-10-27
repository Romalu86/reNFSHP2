#include "gui_widgets.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets {
using game::runtime::invoke;

ImageResource *construct_image_resource(ImageResource *resource) {
    engine::construct_rectangle(&resource->bounds);
    engine::construct_point(&resource->origin);
    resource->methods = image_resource_methods;
    resource->image = 0;
    return resource;
}

void clear_image_resource(ImageResource *resource) {
    if (resource->image && engine_object)
        invoke(engine_object, 120 / 4, resource->image);
    resource->image = 0;
    resource->bounds.top = engine::default_bounds.top;
    resource->bounds.left = engine::default_bounds.left;
    resource->bounds.height = engine::default_bounds.height;
    resource->bounds.width = engine::default_bounds.width;
    resource->origin.x = engine::default_point.x;
    resource->origin.y = engine::default_point.y;
}

void destroy_image_resource(ImageResource *resource) {
    resource->methods = image_resource_methods;
    clear_image_resource(resource);
    engine::destroy_point(&resource->origin);
    engine::destroy_rectangle(&resource->bounds);
    resource->methods = reinterpret_cast<const Method *>(gui::base_methods);
}

static ImageResource *__fastcall delete_image_resource(ImageResource *resource, void *,
                                                       Dword flags) {
    destroy_image_resource(resource);
    if (flags & 1)
        release(resource);
    return resource;
}
const Method image_resource_methods[] = {
    reinterpret_cast<Method>(delete_image_resource)};

ImageResource *copy_image_resource(ImageResource *resource, const ImageResource *source) {
    if (resource == source || resource->image == source->image)
        return resource;
    const Dword image = source->image;
    clear_image_resource(resource);
    Object *engine = engine_object;
    if (image && engine) {
        resource->image = invoke<Dword>(engine, 108 / 4, image);
        if (resource->image)
            invoke(engine, 116 / 4, resource->image, &resource->bounds, &resource->origin);
    }
    return resource;
}

ImageResource *copy_construct_image_resource(ImageResource *resource, const ImageResource *source) {
    construct_image_resource(resource);
    try {
        copy_image_resource(resource, source);
    } catch (...) {
        engine::destroy_point(&resource->origin);
        engine::destroy_rectangle(&resource->bounds);
        resource->methods = reinterpret_cast<const Method *>(gui::base_methods);
        throw;
    }
    return resource;
}

Dword load_image_resource(ImageResource *resource, const String *name) {
    clear_image_resource(resource);
    Object *engine = engine_object;
    if (!engine)
        return 13;
    resource->image = invoke<Dword>(engine, 104 / 4, name);
    if (!resource->image)
        return 9;
    return invoke<Dword>(engine, 116 / 4, resource->image, &resource->bounds, &resource->origin)
               ? 0
               : 4;
}

Dword bind_image_resource(ImageResource *resource, Dword texture, int index) {
    clear_image_resource(resource);
    Object *engine = engine_object;
    if (!engine)
        return 13;
    resource->image = invoke<Dword>(engine, 144 / 4, texture, index);
    return resource->image && invoke<Dword>(engine, 116 / 4, resource->image, &resource->bounds,
                                           &resource->origin)
               ? 0
               : 4;
}

Dword create_image_resource(ImageResource *resource, Dword first, Dword second) {
    clear_image_resource(resource);
    Object *engine = engine_object;
    if (!engine)
        return 13;
    resource->image = invoke<Dword>(engine, 100 / 4, first, second);
    return resource->image && invoke<Dword>(engine, 116 / 4, resource->image, &resource->bounds,
                                           &resource->origin)
               ? 0
               : 4;
}

String *image_resource_name(ImageResource *resource, String *out) {
    if (resource->image && engine_object)
        return invoke<String *>(engine_object, 112 / 4, out, resource->image);
    return string_construct_copy(out, &lifecycle::construction::unresolved::shared_string);
}

Dword read_image_resource(void *configuration, const char *key, ImageResource *resource) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        load_image_resource(resource, value);
        return 1;
    }
    clear_image_resource(resource);
    return 0;
}

Dword write_image_resource(void *configuration, const char *key, ImageResource *resource) {
    String name;
    image_resource_name(resource, &name);
    try {
        const char *text = string_ascii(&name);
        String value;
        string_construct_ascii(&value, text);
        Dword result;
        try {
            result = invoke<Dword>(configuration, 7, key, &value, "SHAPE", Dword(0),
                                  Dword(text == nullptr));
        } catch (...) {
            string_destruct(&value);
            throw;
        }
        string_destruct(&value);
        string_destruct(&name);
        return result;
    } catch (...) {
        string_destruct(&name);
        throw;
    }
}
} // namespace nfs::gui::widgets
