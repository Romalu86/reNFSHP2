#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/network/session/network_virtual.hpp"
namespace nfs::game::world {

void configure_vehicle_detail(void *owner) {
    Dword detail = at<Byte>(owner, 81296) ? 3 : at<Dword>(subsystem(72), 2384);
    if (detail <= 4) {
        switch (detail) {
        case 0:
            at<Dword>(owner, 81272) = 0;
            at<Dword>(owner, 81276) = 0;
            at<Dword>(owner, 81288) = 2;
            at<Dword>(owner, 81284) = 0;
            at<Dword>(owner, 81280) = at<Dword>(owner, 81292) = 0;
            break;
        case 1:
            at<Dword>(owner, 81272) = at<Dword>(owner, 81276) = at<Dword>(owner, 81288) = 1;
            at<Dword>(owner, 81284) = 0;
            at<Dword>(owner, 81280) = at<Dword>(owner, 81292) = 0;
            break;
        case 2:
            at<Dword>(owner, 81272) = at<Dword>(owner, 81276) = at<Dword>(owner, 81284) = 1;
            at<Dword>(owner, 81288) = 0;
            at<Dword>(owner, 81280) = at<Dword>(owner, 81292) = 0;
            break;
        default:
            at<Dword>(owner, 81272) = detail == 4 ? 2 : 1;
            at<Dword>(owner, 81280) = at<Dword>(owner, 81284) = at<Dword>(owner, 81276) =
                at<Dword>(owner, 81292) = 1;
            at<Dword>(owner, 81288) = 0;
            break;
        }
    }
    at<Dword>(owner, 81312) = at<Dword>(owner, 81284) ? 1120403456 : 1115815936;
}
namespace {
void *create_renderables() {
    auto *cookie = static_cast<Dword *>(files::archives::allocate_entry(
        65028, "VEHICLE_MANAGER_VEHICLE_LIST", lifecycle::cars_pool));
    if (!cookie)
        return nullptr;
    *cookie = 16;
    try {
        lifecycle::construction::construct_array(cookie + 1, 4064, 16,
                                                 pending::construct_vehicle_renderable,
                                                 pending::destroy_vehicle_renderable);
    } catch (...) {
        lifecycle::construction::network::entries::release_payload(cookie);
        throw;
    }
    return cookie + 1;
}
void delete_renderables(void *owner, void *list) {
    int count = at<int>(owner, 81304);
    if (count > 0) {
        for (int index = 0; index < count; ++index)
            pending::unload_vehicle_renderable(offset(list, index * 4064));
        at<Dword>(owner, 81304) = 0;
    }
    lifecycle::construction::objects::destroy_array(list, 4064, static_cast<int *>(list)[-1],
                                                    pending::destroy_vehicle_renderable);
    lifecycle::construction::network::entries::release_payload(static_cast<Byte *>(list) - 4);
}
} // namespace

void enter_vehicle_frontend(void *owner) {
    configure_vehicle_detail(owner);
    pending::load_vehicle_shared_textures(offset(owner, 78600));
    pending::initialize_vehicle_shader_parameters();
    void *list = at<void *>(owner, 81268);
    if (list) {
        kernel::print::check_console_write(
            "%s(%d) Warning: FrontEndInit called inappropriately\r\n",
            "C:\\HP2\\Net\\Patch1\\Code\\Source\\Game_Objects\\car\\vehiclemananger.cpp", 261);
        if (list)
            return;
    }
    at<void *>(owner, 81268) = create_renderables();
}

void leave_vehicle_frontend(void *owner) {
    void *list = at<void *>(owner, 81268);
    if (!list)
        kernel::print::check_console_write(
            "%s(%d) Warning: FrontEndDeInit called inappropriately\r\n",
            "C:\\HP2\\Net\\Patch1\\Code\\Source\\Game_Objects\\car\\vehiclemananger.cpp", 284);
    else
        delete_renderables(owner, list);
    at<void *>(owner, 81268) = nullptr;
    pending::release_vehicle_shader_parameters();
    runtime::release_vehicle_assets(reinterpret_cast<Dword *>(offset(owner, 78600)));
}

void load_vehicle_level(void *owner) {
    configure_vehicle_detail(owner);
    pending::load_vehicle_shared_textures(offset(owner, 78600));
    pending::initialize_vehicle_shader_parameters();
    at<void *>(owner, 81268) = create_renderables();
}

void unload_vehicle_level(void *owner) {
    if (void *list = at<void *>(owner, 81268))
        delete_renderables(owner, list);
    at<void *>(owner, 81268) = nullptr;
    pending::release_vehicle_shader_parameters();
    runtime::release_vehicle_assets(reinterpret_cast<Dword *>(offset(owner, 78600)));
}

void reset_vehicle_level(void *owner) {
    int count = at<int>(owner, 81304);
    Byte *object = at<Byte *>(owner, 81268);
    while (count-- > 0) {
        pending::reset_vehicle_renderable(object);
        object += 4064;
    }
}

void initialize_vehicle_catalog(void *owner) {
    pending::load_vehicle_catalog(offset(owner, 4));
}
} // namespace nfs::game::world
