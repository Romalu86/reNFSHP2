#include "vehicle_renderable_resources.hpp"
#include "vehicle_attachment_transforms.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::game::world::dependency::pending {

void load_vehicle_renderable(void *owner, Dword catalog, Dword requested_skin) {
    using namespace vehicle_render;
    world::configure_vehicle_detail(subsystem(52));
    at<Dword>(owner, 3956) = requested_skin;
    at<Dword>(owner, 3952) = catalog;
    const char *resource = catalogue_resource(offset(subsystem(52), 4), catalog);
    at<const char *>(owner, 3964) = resource;
    void *entry = offset(subsystem(52), 8 + catalog * 1228);
    at<Dword>(owner, 3948) = at<Byte>(entry, 32) ? 1 : 0;
    std::memcpy(offset(owner, 3980), offset(entry, 1160), 12);
    std::memcpy(offset(owner, 3992), offset(entry, 1172), 12);
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    float extents[3];
    extents[0] = float((X(at<float>(owner, 3992)) - at<float>(owner, 3980)) * .5f);
    std::memcpy(offset(owner, 4004), offset(entry, 1184), 12);
    extents[1] = float((X(at<float>(owner, 3996)) - at<float>(owner, 3984)) * .5f);
    std::memcpy(offset(owner, 4016), offset(entry, 1196), 12);
    extents[2] = float((X(at<float>(owner, 4000)) - at<float>(owner, 3988)) * .5f);
    std::memcpy(offset(owner, 4028), extents, 12);
    ResourceGroup group{};
    group.catalog = Dword(-1);
    acquire_resource_group(offset(subsystem(52), 78600), catalog, resource, &group);
    void *skin = acquire_skin(offset(subsystem(52), 78600), catalog, requested_skin, resource);
    at<void *>(owner, 996) = skin;
    if (!skin) {
        at<Dword>(owner, 3956) = 0;
        at<void *>(owner, 996) =
            acquire_skin(offset(subsystem(52), 78600), catalog, 0, resource);
    }
    at<Dword>(owner, 1000) = 0;
    void *body = offset(owner, 1004);
    at<Dword>(owner, 3132) = requested_skin;
    bind_body_skin(body, catalog, at<Dword>(owner, 3956));
    world::invoke(body, 0, reinterpret_cast<Dword *>(&group.images),
                  reinterpret_cast<Dword>(group.car), reinterpret_cast<Dword>(group.shadow),
                  at<Dword>(owner, 996), at<Dword>(owner, 1000), group.references - 1);
    bind_body_configuration(body, at<const char *>(owner, 3964));
    at<Dword>(owner, 2988) = at<Dword>(owner, 3948);
    at<void *>(owner, 3116) = owner;
    bind_shadow_extents(body, catalog, reinterpret_cast<float *>(offset(owner, 4028)));
    Dword index = catalogue_index(offset(subsystem(52), 4), at<const char *>(owner, 3964));
    at<Byte>(owner, 3969) =
        catalogue_addons(offset(subsystem(52), 4), index, at<Dword>(owner, 3956));
    if (at<Byte>(owner, 3969))
        enable_body_addons(body);
    void *attachments = offset(owner, 3480);
    vehicle_render::pending::load_attachments(attachments, group.skeleton);
    bind_shadow_owner(offset(owner, 3460), 0);
    at<void *>(owner, 3460) = attachments;
    at<float>(owner, 4040) = catalogue_trunk_bounce(offset(subsystem(52), 4), catalog);
    at<float>(owner, 4044) = catalogue_hood_bounce(offset(subsystem(52), 4), catalog);
    at<float>(owner, 4048) = catalogue_wheel_speed(offset(subsystem(52), 4), catalog);
    if (at<Dword>(subsystem(52), 81280) == 1 &&
        vehicle_render::pending::has_damage_bank(at<const char *>(owner, 3964)) &&
        !at<Dword>(entry, 1140)) {
        at<void *>(owner, 72) = attachments;
        vehicle_render::pending::load_damage(owner, at<const char *>(owner, 3964),
                                             reinterpret_cast<Dword *>(offset(owner, 3980)),
                                             reinterpret_cast<Dword *>(offset(owner, 3992)));
    }
    at<Dword>(owner, 3472) = at<Dword>(entry, 1156);
    at<Dword>(owner, 3476) = at<Dword>(entry, 1152);
    if (!at<Byte>(subsystem(52), 81296) && at<Dword>(owner, 3948) != 1)
        rebind_vehicle_character(owner);
    vehicle_render::pending::load_lights(offset(owner, 80), at<const char *>(owner, 3964),
                                         attachments);
    void *colour =
        catalogue_skin(offset(subsystem(52), 4), at<Dword>(owner, 3952), at<Dword>(owner, 3956));
    at<Dword>(owner, 4056) = 4278190080u | (Dword(at<Byte>(colour, 10)) << 16) |
                             (Dword(at<Byte>(colour, 9)) << 8) | at<Byte>(colour, 8);
}

void rebind_vehicle_character(void *owner) {
    using namespace vehicle_render;
    if (at<Dword>(owner, 3948) == 1)
        return;
    void *attachments = offset(owner, 3480);
    if (!attachment_enabled(attachments, 40))
        return;
    vehicle_render::pending::initialize_attachment_root(attachments);
    Dword matrix[16];
    lifecycle::construction::identity_matrix(matrix);
    std::memcpy(offset(at<void *>(owner, 3888), 48), matrix, 64);
    vehicle_render::pending::evaluate_attachments(attachments);
    Byte left_side = at<Byte>(subsystem(52), at<Dword>(owner, 3952) * 1228 + 1156);
    vehicle_render::pending::bind_driver(offset(owner, 856), at<Dword>(owner, 3956) == 15,
                                         left_side, attachments, at<Dword>(owner, 3976));
}
} // namespace nfs::game::world::dependency::pending
