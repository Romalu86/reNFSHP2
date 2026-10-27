#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../rendering/vehicle_attachment_transforms.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {

long double accumulated_body_damage(void *damage) {
    return at<float>(damage, 8);
}
namespace {
int multiply_percentage(Dword setting, int value) {
    Dword product = setting * Dword(value);
    int signed_product;
    std::memcpy(&signed_product, &product, 4);
    return signed_product / 100;
}
} // namespace

void update_body_damage(void *car) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    float damage = float(accumulated_body_damage(bytes(car) + 7952));
    Extended reference(at<float>(car, 24), format);
    const bool below_half = damage < .5f;
    Extended damage_amount = below_half ? reference * reference * 20000.0f
                                        : reference * Extended(damage, format) * 30000.0f;
    int amount = static_cast<int>(damage_amount.truncate_to_integer());
    int first = multiply_percentage(network::packets::rule_value(system(72), 53), amount);
    int second = multiply_percentage(network::packets::rule_value(system(72), 53), amount);
    first = multiply_percentage(network::packets::rule_value(system(72), 56), first);
    second = multiply_percentage(network::packets::rule_value(system(72), 56), second);
    if (at<int>(car, 7996) <= first)
        at<int>(car, 7996) = first;
    if (at<int>(car, 8000) <= second)
        at<int>(car, 8000) = second;
    if (!below_half) {
        Dword rotation[9], position[3];
        world::vehicle_render::read_vehicle_attachment(at<void *>(car, 3372), 37, rotation,
                                                       position);
    }
}
} // namespace nfs::game::runtime::physics::contacts_pending
