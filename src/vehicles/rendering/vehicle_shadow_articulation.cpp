#include "vehicle_renderable_frame.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::actor_resources {
void actor_axis_rotation(float *, float, float, float, float);
namespace animation_runtime {
void decompose_animation_matrix(const float *, float *, float *);
}
} // namespace nfs::game::world::actor_resources
namespace nfs::game::runtime::physics::contacts_pending {
Byte wheel_is_punctured(void *, Byte);
}
namespace nfs::game::world::vehicle_render {
using kernel::numeric::Extended;

float *attachment_pose(void *attachments, Dword index) {
    const Dword node =
        pending::skeleton_node_index(at<void *>(attachments, 408), attachment_names[index]);
    return reinterpret_cast<float *>(reinterpret_cast<Byte *>(animation_pose_scratch()) +
                                     node * 48);
}

void shadow_pose_matrix(float *matrix, float sx, float sy, float sz, float qx, float qy, float qz,
                        float qw, float x, float y, float z) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    matrix[3] = matrix[7] = 0;
    const auto twice_x = X(qx) + qx, twice_y = X(qy) + qy;
    const float twice_z = float(X(qz) + qz);
    const float xw = float(twice_x * qw), yw = float(X(qw) * twice_y),
                zw = float(X(twice_z) * qw);
    const float xx = float(twice_x * qx), xy = float(X(qx) * twice_y),
                xz = float(X(twice_z) * qx);
    const auto yy = twice_y * qy, yz = X(twice_z) * qy, zz = X(twice_z) * qz;
    matrix[0] = float((X(1) - (zz + yy)) * sx);
    matrix[4] = float((X(xy) - zw) * sy);
    matrix[8] = float((X(xz) + yw) * sz);
    matrix[1] = float((X(xy) + zw) * sx);
    matrix[5] = float((X(1) - (zz + xx)) * sy);
    matrix[9] = float((yz - xw) * sz);
    matrix[2] = float((X(xz) - yw) * sx);
    matrix[6] = float((yz + xw) * sy);
    matrix[10] = float((X(1) - (yy + xx)) * sz);
    matrix[11] = 0;
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
    matrix[15] = 1;
}
// This entry replaces only the nine rotational elements.
void shadow_axis_rotation(float *matrix, float degrees, float x, float y, float z) {
    float rotation[16];
    actor_resources::actor_axis_rotation(rotation, degrees, x, y, z);
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 3; ++column)
            matrix[row * 4 + column] = rotation[row * 4 + column];
}
static void save_shadow_pose(float *pose, const float *matrix) {
    float quaternion[4], translation[4];
    actor_resources::animation_runtime::decompose_animation_matrix(matrix, quaternion, translation);
    pose[4] = quaternion[0];
    pose[5] = quaternion[1];
    pose[6] = quaternion[2];
    pose[8] = translation[0];
    pose[7] = quaternion[3];
    pose[9] = translation[1];
    pose[10] = translation[2];
    pose[11] = translation[3];
}

void articulate_shadow_wheel(void *wheel, float *pose, float spin, float steer, float pitch,
                             float roll, float suspension, float radius, Byte caliper) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float puncture_angle = 0;
    if (!at<Byte>(subsystem(52), 81296)) {
        const Dword wheel_index = at<Dword>(wheel, 12);
        if (runtime::physics::contacts_pending::wheel_is_punctured(
                offset(at<void *>(wheel, 0), 7952), Byte(wheel_index))) {
            puncture_angle = 4;
            spin = float(Extended::integer(int(wheel_index), format) * .125f + spin);
            if (!(suspension < 0) || std::isnan(suspension))
                suspension = 0;
        }
    } else {
        suspension = 0;
        spin = pitch = roll = 1;
    }
    if (void *limits = at<void *>(wheel, 4)) {
        const float lower = at<float>(limits, 48), upper = at<float>(limits, 52);
        if (suspension < lower)
            suspension = lower;
        else if (upper < suspension)
            suspension = upper;
    }
    suspension = float(X(suspension) + radius);
    float articulation[16], rotation[16], base[16], combined[16];
    if (caliper)
        graphics::math::diagonal(articulation, 1, 1, 1, 1);
    else
        actor_resources::actor_axis_rotation(articulation, spin, 1, 0, 0);
    if (puncture_angle != 0 || std::isnan(puncture_angle)) {
        actor_resources::actor_axis_rotation(rotation, puncture_angle, 0, 0, 1);
        graphics::math::multiply(rotation, articulation, combined);
        graphics::math::copy(articulation, combined);
    }
    actor_resources::actor_axis_rotation(rotation, -steer, 0, 1, 0);
    graphics::math::append(articulation, rotation);
    actor_resources::actor_axis_rotation(rotation, roll, 0, 0, 1);
    graphics::math::append(articulation, rotation);
    actor_resources::actor_axis_rotation(rotation, -pitch, 1, 0, 0);
    graphics::math::append(articulation, rotation);
    graphics::math::translation(rotation, 0, suspension, 0);
    graphics::math::append(articulation, rotation);
    shadow_pose_matrix(base, pose[0], pose[1], pose[2], pose[4], pose[5], pose[6], pose[7], pose[8],
                       pose[9], pose[10]);
    shadow_axis_rotation(base, 0, 0, 1, 0);
    graphics::math::multiply(articulation, base, combined);
    graphics::math::copy(base, combined);
    save_shadow_pose(pose, base);
}

void update_shadow_attachments(void *shadow) {
    void *attachments = at<void *>(shadow, 0);
    float *poses[8];
    constexpr Dword indices[8] = {2, 3, 5, 6, 12, 13, 15, 16};
    for (unsigned i = 0; i < 8; ++i)
        poses[i] = i < 4 || attachment_enabled(attachments, indices[i])
                       ? attachment_pose(attachments, indices[i])
                       : nullptr;
    if (at<Byte>(subsystem(52), 81296)) {
        at<Dword>(at<void *>(shadow, 4), 1964) = 1064514355;
        at<Dword>(at<void *>(shadow, 4), 2260) = 1064514355;
    }
    const auto format = kernel::numeric::current_format();
    auto degrees = [&](float value) { return float(Extended(value, format) * 360.f); };
    void *car = at<void *>(shadow, 4);
    const float front_spin = degrees(at<float>(car, 1624)),
                rear_spin = degrees(at<float>(car, 1620));
    const float pitch = degrees(at<float>(car, 364)), roll = degrees(at<float>(car, 368));
    constexpr Dword wheels[4] = {1948, 2244, 2836, 2540};
    for (unsigned i = 0; i < 8; ++i)
        if (poses[i]) {
            car = at<void *>(shadow, 4);
            const Dword wheel = wheels[i % 4];
            articulate_shadow_wheel(offset(car, wheel), poses[i],
                                    i >= 4  ? 0
                                    : i < 2 ? front_spin
                                            : rear_spin,
                                    degrees(at<float>(car, wheel + 16)), pitch, roll,
                                    at<float>(car, wheel + 252),
                                    at<float>(shadow, i % 4 < 2 ? 12 : 16), Byte(i >= 4));
        }
}

void update_vehicle_shadow(void *shadow) {
    if (at<Dword>(shadow, 8) == 0 || at<Dword>(shadow, 8) == 1)
        update_shadow_attachments(shadow);
}
static void rotate_shadow_panel(float *pose, float turns) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    const float cosine = float(runtime::turn_cosine(turns));
    const auto sine = Extended::from_double(double(runtime::turn_sine(turns)), format);
    float rotation[16];
    rotation[0] = 1;
    rotation[3] = rotation[7] = rotation[11] = rotation[12] = rotation[13] = rotation[14] = 0;
    rotation[15] = 1;
    const float zero_cosine = float(X(cosine) * 0.f);
    const auto zero_sine = sine * 0.f;
    rotation[8] = float(zero_sine) + zero_cosine;
    rotation[4] = float(zero_sine - X(zero_cosine));
    rotation[1] = float(zero_sine + zero_cosine);
    rotation[2] = float(zero_sine) - zero_cosine;
    rotation[5] = float(X(1) - float((sine + sine) * sine));
    rotation[6] = float(X(cosine) * (sine + sine));
    rotation[9] = -rotation[6];
    rotation[10] = rotation[5];
    const float scale_x = pose[0], scale_y = pose[1], scale_z = pose[2], qx = pose[4], qy = pose[5],
                qz = pose[6], qw = pose[7];
    const float xx2 = float(X(qx) + qx), yy2 = float(X(qy) + qy), zz2 = float(X(qz) + qz);
    const float xw = float(X(xx2) * qw), yw = float(X(yy2) * qw), zw = float(X(zz2) * qw);
    const float xx = float(X(xx2) * qx), xy = float(X(yy2) * qx), xz = float(X(zz2) * qx),
                yy = float(X(yy2) * qy), yz = float(X(zz2) * qy), zz = float(X(zz2) * qz);
    float base[16] = {float((X(1) - (X(yy) + zz)) * scale_x),
                      float((X(xy) + zw) * scale_x),
                      float((X(xz) - yw) * scale_x),
                      0,
                      float((X(xy) - zw) * scale_y),
                      float((X(1) - (X(zz) + xx)) * scale_y),
                      float((X(yz) + xw) * scale_y),
                      0,
                      float((X(xz) + yw) * scale_z),
                      float((X(yz) - xw) * scale_z),
                      float((X(1) - (X(yy) + xx)) * scale_z),
                      0,
                      pose[8],
                      pose[9],
                      pose[10],
                      1};
    float combined[16];
    graphics::math::multiply(rotation, base, combined);
    graphics::math::copy(base, combined);
    save_shadow_pose(pose, base);
}

void rotate_shadow_trunk(float *pose, float turns) {
    turns = -turns;
    if (!(turns < 0) || std::isnan(turns))
        turns = 0;
    rotate_shadow_panel(pose, turns);
}

void rotate_shadow_hood(float *pose, float turns) {
    if (turns <= 0)
        turns = 0;
    rotate_shadow_panel(pose, turns);
}

void update_trunk_shadow(void *shadow) {
    void *attachments = at<void *>(shadow, 0);
    if (!attachment_enabled(attachments, 44))
        return;
    float *pose = attachment_pose(attachments, 44);
    if (!pose)
        return;
    void *car = at<void *>(shadow, 4);
    const auto format = kernel::numeric::current_format();
    const float middle = float(Extended(at<float>(car, 1636), format) + at<float>(car, 1632));
    rotate_shadow_trunk(pose, float((Extended(middle, format) + middle + at<float>(car, 1640) +
                                     at<float>(car, 1628)) *
                                    0.16666667163372039794921875f));
}

void update_hood_shadow(void *shadow) {
    void *attachments = at<void *>(shadow, 0);
    if (!attachment_enabled(attachments, 38))
        return;
    float *pose = attachment_pose(attachments, 38);
    if (!pose)
        return;
    void *car = at<void *>(shadow, 4);
    const auto format = kernel::numeric::current_format();
    const float middle = float(Extended(at<float>(car, 1668), format) + at<float>(car, 1664));
    rotate_shadow_hood(pose, float((Extended(middle, format) + middle + at<float>(car, 1672) +
                                    at<float>(car, 1660)) *
                                   0.16666667163372039794921875f));
}

void prepare_renderable_panels(void *renderable) {
    void *shadow = offset(renderable, 3460);
    update_vehicle_shadow(shadow);
    if (!at<Byte>(renderable, 20))
        return;
    const auto format = kernel::numeric::current_format();
    if (at<float>(renderable, 4044) != 0 || std::isnan(at<float>(renderable, 4044))) {
        const float damage = damage_amount(renderable, 0);
        if (Extended(damage_limit(renderable, 0), format) * .7f < Extended(damage, format)) {
            runtime::physics::animate_hood(
                at<void *>(renderable, 3976),
                float(Extended(at<float>(renderable, 4044), format) * 0.00277777784503996372222900390625f));
            update_hood_shadow(shadow);
        }
    }
    if (at<float>(renderable, 4040) != 0 || std::isnan(at<float>(renderable, 4040))) {
        const float damage = damage_amount(renderable, 7);
        if (Extended(damage_limit(renderable, 7), format) * .9f < Extended(damage, format)) {
            runtime::physics::animate_trunk(
                at<void *>(renderable, 3976),
                float(Extended(at<float>(renderable, 4040), format) * 0.00277777784503996372222900390625f));
            update_trunk_shadow(shadow);
        }
    }
}
} // namespace nfs::game::world::vehicle_render
