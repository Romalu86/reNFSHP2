#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "gui_widget_image_list.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdlib>
namespace nfs::gui::widgets::family {
using game::runtime::invoke;
Container image_items{};
Dword image_item_serial{};
unsigned char image_items_destroyed{};

static void destroy_image_item_registry() {
    if (!(image_items_destroyed & 1)) {
        image_items_destroyed |= 1;
        container_destruct(&image_items);
    }
}
 
static void initialize_image_item_registry() {
    container_construct(&image_items);
    std::atexit(destroy_image_item_registry);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_image_item_registry, ".CRT$XCU_NFS_00690700",
                       initialize_image_item_registry);
} // namespace

int compare_image_item(const void *value, const void *key) {
    const Dword id = static_cast<const ImageItem *>(value)->id;
    const Dword wanted = reinterpret_cast<Dword>(key);
    return id < wanted ? -1 : id > wanted ? 1 : 0;
}

ImageItem *find_image_item(Dword id) {
    return id ? static_cast<ImageItem *>(
                    container_find(&image_items, compare_image_item, reinterpret_cast<void *>(id)))
              : nullptr;
}

ImageItem *construct_image_item(ImageItem *item, int frame, Dword data, Dword enabled) {
    item->methods = reinterpret_cast<const Method *>(base_methods);
    construct_image_resource(&item->resource);
    image_item_serial += 4;
    item->id = image_item_serial;
    item->data = data;
    item->enabled = enabled;
    item->methods = image_item_methods;
    item->frame = frame;
    try {
        container_sorted_insert(&image_items, compare_image_item,
                                reinterpret_cast<void *>(item->id), item, nullptr);
    } catch (...) {
        destroy_image_resource(&item->resource);
        item->methods = reinterpret_cast<const Method *>(base_methods);
        throw;
    }
    return item;
}

void destroy_image_item(ImageItem *item) {
    item->methods = image_item_methods;
    container_detach(&image_items, item);
    destroy_image_resource(&item->resource);
    item->methods = reinterpret_cast<const Method *>(base_methods);
}

static ImageItem *__fastcall delete_image_item(ImageItem *item, void *, Dword flags) {
    destroy_image_item(item);
    if (flags & 1)
        release(item);
    return item;
}
const Method image_item_methods[] = {reinterpret_cast<Method>(delete_image_item)};

Dword ensure_image_item_frame(ImageItem *item, ImageSequence *sequence) {
    if (item->resource.image)
        return 1;
    return sequence->texture && item->frame >= 0 && item->frame < int(sequence->frames) &&
           active_engine() && !bind_image_resource(&item->resource, sequence->texture, item->frame);
}

void release_image_item_frame(ImageItem *item) {
    if (item->resource.image)
        clear_image_resource(&item->resource);
}

Dword image_item_enabled(Dword id) {
    auto *item = find_image_item(id);
    return item ? item->enabled : 0;
}

Dword image_item_handle(Dword id) {
    auto *item = find_image_item(id);
    return item ? item->resource.image : 0;
}

Dword append_image_frame(Widget *widget, int frame, Dword data, Dword enabled) {
    auto *item = static_cast<ImageItem *>(allocate_flags(sizeof(ImageItem), 0, "GImageItem"));
    if (!item)
        return 0;
    try {
        construct_image_item(item, frame, data, enabled);
    } catch (...) {
        release_flags(item, 0, "GImageItem");
        throw;
    }
    if (container_append(&image_list(widget)->items, item, dispose_object)) {
        invoke(item, 0, Dword(1));
        return 0;
    }
    refresh_image_frames(widget);
    return item->id;
}

Dword append_image_frame_name(Widget *widget, const char *name, Dword data, Dword enabled) {
    if (!image_list(widget)->sequence.texture)
        return 0;
    auto *render = active_engine();
    if (!render)
        return 0;
    String frame_name;
    string_construct_ascii(&frame_name, name);
    Dword id;
    try {
        const int frame =
            invoke<int>(render, 38, image_list(widget)->sequence.texture, &frame_name);
        id = append_image_frame(widget, frame, data, enabled);
    } catch (...) {
        string_destruct(&frame_name);
        throw;
    }
    string_destruct(&frame_name);
    return id;
}
} // namespace nfs::gui::widgets::family
