#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {

void update_car_orientation_frame(void *car) {
    void *body = at<void *>(car, 1768);
    at<float>(car, 452) = 1;
    const float longitudinal = at<float>(car, 3156);
    if (longitudinal > 0)
        at<float>(car, 452) = float(drive::retained(1) - at<float>(car, 628));
    else if (longitudinal < 0)
        at<float>(car, 452) = float(drive::retained(1) - at<float>(car, 624));
    float factor = at<float>(car, 452);
    if (factor < 0)
        factor = 0;
    else if (factor > 1)
        factor = 1;
    at<float>(car, 452) = factor;
    if (at<int>(car, 448) < 2) {
        const float length = at<float>(body, 128), width = at<float>(body, 120);
        float maximum = at<float>(car, 652);
        if (maximum < at<float>(car, 648))
            maximum = at<float>(car, 648);
        float upright = at<float>(body, 708);
        if (upright < 0)
            upright = 0;
        else if (upright > 1)
            upright = 1;
        float normal[3];
        matrices::transform_vectors(1, vector_at(body, 520), vector_at(body, 72), normal);
        const float abs_y = float(drive::absolute(drive::retained(at<float>(body, 708))));
        const float roll =
            float(drive::direction(at<float>(body, 696), abs_y) * upright);
        const float pitch =
            float(drive::direction(at<float>(body, 720), abs_y) * upright);
        at<float>(car, 456) =
            float(drive::retained(pitch) - drive::direction(normal[2], normal[1]) +
                  at<float>(car, 360));
        at<float>(car, 460) =
            float(drive::retained(roll) - drive::direction(normal[0], normal[1]));
        const float roll_height = float(drive::absolute(drive::sine(at<float>(car, 368)) * width));
        auto height = drive::absolute(drive::sine(at<float>(car, 364)) * length);
        if (!(drive::retained(roll_height) < height))
            height = drive::retained(roll_height);
        const auto minimum_height = drive::retained(upright) * maximum * .66f;
        if (height < minimum_height)
            height = drive::retained(float(minimum_height));
        const auto pitch_change = -drive::retained(at<float>(car, 364)) * 0.01666666753590106964111328125f;
        at<float>(car, 464) = float(pitch_change);
        const float roll_change = float(-drive::retained(at<float>(car, 368)) * 0.01666666753590106964111328125f);
        at<float>(car, 468) = roll_change;
        const float height_change = float((height - at<float>(car, 372)) * 0.01666666753590106964111328125f);
        at<float>(car, 472) = height_change;
        const auto requested_pitch = drive::retained(at<float>(car, 364)) + pitch_change;
        const float stored_pitch = float(requested_pitch);
        at<float>(car, 364) = stored_pitch;
        const float requested_roll = float(drive::retained(at<float>(car, 368)) + roll_change);
        at<float>(car, 368) = requested_roll;
        const float lower_pitch = -at<float>(car, 264);
        const float new_pitch = requested_pitch < drive::retained(lower_pitch) ? lower_pitch
                                : at<float>(car, 268) < stored_pitch ? at<float>(car, 268)
                                                                     : stored_pitch;
        at<float>(car, 364) = new_pitch;
        const float lower_roll = -at<float>(car, 272);
        const float new_roll = requested_roll < lower_roll ? lower_roll
                               : at<float>(car, 272) < requested_roll ? at<float>(car, 272)
                                                                        : requested_roll;
        at<float>(car, 368) = new_roll;
        auto new_height = drive::retained(at<float>(car, 372)) + height_change;
        at<float>(car, 372) = float(new_height);
        if (new_height < drive::retained(0))
            new_height = drive::retained(0);
        else if (drive::retained(maximum) < new_height)
            new_height = drive::retained(maximum);
        at<float>(car, 372) = float(new_height);
    } else {
        at<Dword>(car, 464) = 0;
        at<Dword>(car, 468) = 0;
        at<Dword>(car, 472) = 0;
    }
    float pitch_rotation[9], roll_rotation[9];
    matrices::rotate_x(pitch_rotation, at<float>(car, 364));
    matrices::rotate_z(roll_rotation, at<float>(car, 368));
    matrices::multiply(pitch_rotation, roll_rotation, vector_at(car, 376));
    matrices::multiply(vector_at(car, 376), vector_at(at<void *>(car, 1768), 692),
                       vector_at(car, 376));
    matrices::transpose(vector_at(car, 376), vector_at(car, 412));
}
} // namespace nfs::game::runtime::physics::car_frame
