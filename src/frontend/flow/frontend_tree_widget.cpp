#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include <cstdio>
namespace nfs::game::runtime::particles::pending {
Dword *find_fsh_image(Dword *, const char *);
}
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::system;

void *load_menu_art_file(const char *name, Dword flags) {
    char archive[260];
    std::sprintf(archive, "%s%s", bytes(system(4)) + 2344, "gui\\guiart.viv");
    return files::archives::load(archive, name, flags, 0);
}

TreeWidget *construct_tree_widget(TreeWidget *widget, void *parent) {
    construct_actor_preview(&widget->base, parent);
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_actor_preview(&widget->base); });
    widget->model_images = nullptr;
    gui::string_construct(&widget->resource);
    kernel::lifecycle::Cleanup resource_base_cleanup([=] { widget->resource.methods = gui::base_methods; });
    gui::string_copy(&widget->resource,
                     &lifecycle::construction::unresolved::shared_string);
    resource_base_cleanup.dismiss();
    kernel::lifecycle::Cleanup resource_cleanup([=] { gui::string_destruct(&widget->resource); });
    gui::string_construct(&widget->camera);
    kernel::lifecycle::Cleanup camera_base_cleanup([=] { widget->camera.methods = gui::base_methods; });
    gui::string_copy(&widget->camera,
                     &lifecycle::construction::unresolved::shared_string);
    camera_base_cleanup.dismiss();
    kernel::lifecycle::Cleanup camera_cleanup([=] { gui::string_destruct(&widget->camera); });
    widget->texture_bank = widget->selected_image = nullptr;
    widget->rotation = 0;
    widget->selection = 0;
    widget->camera_depth = 0;
    widget->base.base.base.base.methods = tree_widget_methods.data();
    gui::string_assign(&widget->base.resource, "EventTree2");
    camera_cleanup.dismiss();
    resource_cleanup.dismiss();
    base_cleanup.dismiss();
    return widget;
}

void destroy_tree_widget(TreeWidget *widget) {
    widget->base.base.base.base.methods = tree_widget_methods.data();
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_actor_preview(&widget->base); });
    kernel::lifecycle::Cleanup resource_cleanup([=] { gui::string_destruct(&widget->resource); });
    kernel::lifecycle::Cleanup camera_cleanup([=] { gui::string_destruct(&widget->camera); });
    if (widget->texture_bank)
        gui::application_release(widget->texture_bank);
}

void __fastcall initialize_tree_widget(TreeWidget *widget, void *) {
    initialize_actor_preview(&widget->base, nullptr);
    pending::LocalFrontendString filename;
    gui::string_format_ascii(&filename.value, "%s", bytes(system(4)) + 2604);
    gui::string_append(&filename.value, &widget->camera);
    widget->texture_bank =
        load_menu_art_file(gui::string_ascii(&filename.value), lifecycle::track_pool | 1024);
    widget->model_images = at<void *>(at<void *>(widget->base.actor, 392), 128);
}
 
void __fastcall resume_tree_widget(TreeWidget *, void *) {
}

Dword __fastcall load_tree_widget(TreeWidget *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_string(configuration, "TreeWidget.m_NewTexFile", &widget->camera,
                &lifecycle::construction::unresolved::shared_string);
    read_float(configuration, "TreeWidget.mZoomStopPoint", &widget->camera_depth, 0);
    return load_actor_preview(&widget->base, nullptr, configuration);
}

Dword __fastcall save_tree_widget(TreeWidget *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_text(configuration, "TreeWidget.m_NewTexFile", gui::string_ascii(&widget->camera),
               nullptr);
    write_float(configuration, "TreeWidget.mZoomStopPoint", widget->camera_depth, 2, 0);
    return save_actor_preview(&widget->base, nullptr, configuration);
}

gui::String *tree_event_image_name(gui::String *out, Dword type) {
    const char *name;
    switch (type) {
    case 0:
    case 5:
        name = "fre";
        break;
    case 1:
        name = "knk";
        break;
    case 2:
        name = "chm";
        break;
    case 3:
        name = "tim";
        break;
    case 6:
    case 10:
        name = "cop";
        break;
    case 9:
        name = "cap";
        break;
    case 12:
    case 13:
        name = "fox";
        break;
    default:
        name = "ptp";
    }
    return gui::string_construct_ascii(out, name);
}

gui::String *tree_node_name(gui::String *out, int node) {
    int number;
    switch (node) {
    case 0:
        number = 2;
        break;
    case 1:
        number = 3;
        break;
    case 2:
        number = 4;
        break;
    case 3:
        number = 5;
        break;
    case 4:
        number = 8;
        break;
    case 5:
        number = 6;
        break;
    case 6:
        number = 9;
        break;
    case 7:
        number = 7;
        break;
    default:
        number = int(Dword(node) + 2);
    }
    pending::LocalFrontendString name(&lifecycle::construction::unresolved::shared_string);
    gui::string_format_ascii(&name.value, number < 10 ? "000%d" : "00%d", number);
    return gui::string_construct_copy(out, &name.value);
}

Dword bind_tree_event_image(TreeWidget *widget, int node, Dword type, std::uint8_t locked) {
    gui::String name;
    tree_event_image_name(&name, type);
    struct Owned {
        gui::String *s;
        ~Owned() {
            gui::string_destruct(s);
        }
    } owned{&name};
    if (gui::string_equal(&name, &lifecycle::construction::unresolved::shared_string))
        return 0;
    gui::string_append_ascii(&name, locked == 1 ? "L" : "U");
    widget->selected_image = game::runtime::particles::pending::find_fsh_image(
        static_cast<Dword *>(widget->texture_bank), gui::string_ascii(&name));
    if (widget->selected_image && widget->model_images) {
        gui::String shape;
        tree_node_name(&shape, node);
        try {
            game::world::vehicle_render::bind_model_shape(
                widget->model_images, gui::string_ascii(&shape),
                static_cast<Dword *>(widget->selected_image));
        } catch (...) {
            gui::string_destruct(&shape);
            throw;
        }
        gui::string_destruct(&shape);
        return 1;
    }
    return 0;
}

Dword bind_tree_medal_image(TreeWidget *widget, int node, Dword status) {
    pending::LocalFrontendString name(&lifecycle::construction::unresolved::shared_string);
    if (status == 3)
        gui::string_format_ascii(&name.value, "brnz");
    else if (status == 4)
        gui::string_format_ascii(&name.value, "silv");
    else if (status == 5)
        gui::string_format_ascii(&name.value, "gold");
    if (gui::string_equal(&name.value,
                          &lifecycle::construction::unresolved::shared_string))
        return 0;
    widget->selected_image = game::runtime::particles::pending::find_fsh_image(
        static_cast<Dword *>(widget->texture_bank), gui::string_ascii(&name.value));
    if (widget->selected_image && widget->model_images) {
        gui::String shape;
        tree_node_name(&shape, node);
        try {
            game::world::vehicle_render::bind_model_shape(
                widget->model_images, gui::string_ascii(&shape),
                static_cast<Dword *>(widget->selected_image));
        } catch (...) {
            gui::string_destruct(&shape);
            throw;
        }
        gui::string_destruct(&shape);
        return 1;
    }
    return 0;
}
} // namespace nfs::application::frontend
