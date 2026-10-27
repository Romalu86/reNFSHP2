#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {
namespace {
void transfer_load(void *car, unsigned offset, long double transfer) {
    const float load = float(drive::retained(at<float>(car, offset)) + transfer);
    at<float>(car, offset) = load;
    at<float>(car, offset) = load > 0 ? load : 0;
}
} // namespace

void apply_lateral_weight_transfer(void *car) {
    float rear_length = at<float>(car, 116);
    const float height = float(drive::retained(at<float>(car, 544)) * .9090909f);
    const auto bank = drive::absolute(drive::retained(at<float>(car, 456)));
    if (drive::retained(.25f) < bank)
        return;
    const float threshold = float(drive::retained(gravity) * .02f);
    if (drive::retained(threshold) < drive::absolute(drive::retained(at<float>(car, 3156)))) {
        const auto load = -drive::retained(at<float>(car, 3156)) *
                          (drive::retained(height) / drive::retained(at<float>(car, 528))) *
                          at<float>(car, 520) * .5f;
        const float rear_load = float(-load);
        transfer_load(car, 2180, load);
        transfer_load(car, 2476, load);
        transfer_load(car, 3068, drive::retained(rear_load));
        transfer_load(car, 2772, drive::retained(rear_load));
    }
    const float speed = float(drive::absolute(drive::retained(at<float>(car, 3132))));
    const float lateral = at<float>(car, 3148);
    if (rear_length < at<float>(car, 112))
        rear_length = at<float>(car, 112);
    const auto yaw = drive::absolute(drive::retained(rear_length) * at<float>(car, 3200) * 6.283185482025146484375f);
    if (!(drive::retained(threshold) < drive::absolute(drive::retained(lateral))) || !(drive::retained(6) < yaw + speed))
        return;
    const auto front_mass =
        (drive::retained(at<float>(car, 2476)) + at<float>(car, 2180)) * inverse_gravity;
    const float rear_mass =
        float((drive::retained(at<float>(car, 2772)) + at<float>(car, 3068)) * inverse_gravity);
    auto front_height = drive::retained(height) - drive::retained(at<float>(car, 352)) * .8f;
    if (!(drive::retained(0) < front_height))
        front_height = drive::retained(0);
    const float front_load =
        float((drive::retained(lateral) / drive::retained(at<float>(car, 532))) * front_height *
              front_mass);
    auto rear_height = drive::retained(height) - drive::retained(at<float>(car, 356)) * .8f;
    if (!(drive::retained(0) < rear_height))
        rear_height = drive::retained(0);
    const auto rear_load =
        (drive::retained(lateral) / drive::retained(at<float>(car, 536))) * rear_height * rear_mass;
    const float negative_rear_load = float(-rear_load);
    transfer_load(car, 2180, drive::retained(front_load));
    transfer_load(car, 2476, -drive::retained(front_load));
    transfer_load(car, 3068, rear_load);
    transfer_load(car, 2772, drive::retained(negative_rear_load));
}

void apply_body_roll_weight_transfer(void *car) {
    const auto bank = drive::absolute(drive::retained(at<float>(car, 456)));
    if (drive::retained(.25f) < bank || !at<Dword>(car, 448))
        return;
    float wheelbase = at<float>(car, 116);
    if (wheelbase < at<float>(car, 112))
        wheelbase = at<float>(car, 112);
    const auto metric = drive::absolute(drive::retained(at<float>(car, 3132))) +
                        drive::absolute(drive::retained(wheelbase) * at<float>(car, 3200) * 6.283185482025146484375f);
    const float weight = float((drive::retained(at<float>(car, 520)) - at<float>(car, 524)) * gravity);
    if (!(drive::retained(.25f) < metric))
        return;
    const auto reciprocal = drive::retained(1) / (drive::retained(at<float>(car, 620)) + at<float>(car, 616));
    const float rear_factor = float(drive::retained(at<float>(car, 620)) * reciprocal);
    const float front_factor = float(reciprocal * at<float>(car, 616));
    const float front_roll = float(drive::sine(-at<float>(car, 368)) * rear_factor * weight);
    const auto rear_roll = drive::sine(-at<float>(car, 368)) * front_factor * weight;
    const auto add_if_contact = [&](unsigned wheel, unsigned load, long double change) {
        if (at<Byte>(car, wheel))
            at<float>(car, load) = float(drive::retained(at<float>(car, load)) + change);
    };
    add_if_contact(2204, 2180, drive::retained(front_roll));
    add_if_contact(2500, 2476, -drive::retained(front_roll));
    add_if_contact(3092, 3068, rear_roll);
    add_if_contact(2796, 2772, -rear_roll);
    const float inverse_length = float(drive::retained(1) / drive::retained(at<float>(car, 528)));
    const auto front_pitch =
        drive::sine(-at<float>(car, 364)) * inverse_length * rear_factor * weight * at<float>(car, 112);
    const float stored_front_pitch = float(front_pitch + front_pitch);
    auto rear_pitch = drive::sine(-at<float>(car, 364)) * inverse_length * front_factor * weight *
                      at<float>(car, 116);
    rear_pitch = rear_pitch + rear_pitch;
    add_if_contact(2204, 2180, drive::retained(stored_front_pitch));
    add_if_contact(2500, 2476, drive::retained(stored_front_pitch));
    add_if_contact(3092, 3068, -rear_pitch);
    add_if_contact(2796, 2772, -rear_pitch);
    for (unsigned offset : {2180u, 2476u, 3068u, 2772u})
        if (!(at<float>(car, offset) > 0))
            at<Dword>(car, offset) = 0;
}
} // namespace nfs::game::runtime::physics::car_frame
