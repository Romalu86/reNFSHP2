#include "../screens/frontend_menu_roots.hpp"
#include "../../police/arrest/arrest_cinematic_runtime.hpp"
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::actor_resources::animation_runtime {
void decompose_animation_matrix(const float *, float *, float *);
}
namespace nfs::application::frontend {
using game::runtime::at;
namespace {
int truncate_tree_value(long double value) {
    const Dword bits = Dword(game::runtime::hardware::truncate_integer_native80(&value));
    int result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}

void tree_rotation_matrix(float *matrix, float degrees) {
    const auto radians = static_cast<long double>(3.1415927410125732f) *
                         0.0055555556900799274f * degrees;
    const float angle = float(radians);
    if (radians == 0.0L) {
        lifecycle::construction::identity_matrix(reinterpret_cast<Dword *>(matrix));
        return;
    }
    const auto sine = game::runtime::actor_sine_retained(angle);
    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
    const auto cosine = game::runtime::actor_cosine_retained(angle);
    static const volatile double unit_axis_squared = 1.0;
    const auto reciprocal = 1.0L / __builtin_sqrtl(static_cast<long double>(unit_axis_squared));
    const auto normalized_zero = 0.0L * reciprocal;
    const float stored_zero = float(normalized_zero);
    const float normalized_y = float(reciprocal);
    const auto complement = 1.0L - cosine;
    const auto product_x = static_cast<long double>(stored_zero) * complement;
    const float product_y = float(complement * normalized_y);
    const float product_z = float(complement * normalized_zero);
    const float sine_x = float(sine * stored_zero);
    const float sine_y = float(sine * normalized_y);
    const float sine_z = float(sine * normalized_zero);
    matrix[0] = float(static_cast<long double>(stored_zero) * product_x + cosine);
    matrix[1] = float(static_cast<long double>(normalized_y) * product_x + sine_z);
    matrix[2] = float(product_x * normalized_zero - sine_y);
    matrix[4] = float(static_cast<long double>(product_y) * stored_zero - sine_z);
    matrix[5] = float(static_cast<long double>(product_y) * normalized_y + cosine);
    matrix[6] = float(static_cast<long double>(product_y) * normalized_zero + sine_x);
    matrix[8] = float(static_cast<long double>(product_z) * stored_zero + sine_y);
    matrix[9] = float(static_cast<long double>(product_z) * normalized_y - sine_x);
    matrix[10] = float(static_cast<long double>(product_z) * normalized_zero + cosine);
}

void tree_pose_matrix(float *matrix, const float *pose, const SceneWidget &scene, bool entering) {
    const auto translation_z = entering
                                   ? static_cast<long double>(pose[10]) + scene.camera_origin[2]
                                   : static_cast<long double>(scene.camera_origin[2]) + pose[10];
    float scale_x;
    std::memcpy(&scale_x, pose, sizeof scale_x);
    const auto translation_y = static_cast<long double>(pose[9]) - scene.camera_origin[1];
    float scale_y;
    std::memcpy(&scale_y, pose + 1, sizeof scale_y);
    const auto translation_x = static_cast<long double>(pose[8]) - scene.camera_origin[0];
    float scale_z;
    std::memcpy(&scale_z, pose + 2, sizeof scale_z);
    const auto w = static_cast<long double>(pose[7]);
    const auto z = static_cast<long double>(pose[6]);
    const auto y = static_cast<long double>(pose[5]);
    const auto x = static_cast<long double>(pose[4]);
    const float xx = float(x + x);
    const float yy = float(y + y);
    const float zz = float(z + z);
    const float xw = float(static_cast<long double>(xx) * w);
    const float yw = float(static_cast<long double>(yy) * w);
    const float zw = float(static_cast<long double>(zz) * w);
    const float xs = float(static_cast<long double>(xx) * x);
    const float xy = float(static_cast<long double>(yy) * x);
    const float xz = float(static_cast<long double>(zz) * x);
    const float ys = float(static_cast<long double>(yy) * y);
    const float yz = float(static_cast<long double>(zz) * y);
    const auto zs = static_cast<long double>(zz) * z;
    matrix[0] = float((1.0L - (static_cast<long double>(ys) + zs)) * scale_x);
    matrix[4] = float((static_cast<long double>(xy) - zw) * scale_y);
    matrix[8] = float((static_cast<long double>(xz) + yw) * scale_z);
    matrix[1] = float((static_cast<long double>(xy) + zw) * scale_x);
    matrix[5] = float((1.0L - (zs + xs)) * scale_y);
    matrix[9] = float((static_cast<long double>(yz) - xw) * scale_z);
    matrix[2] = float((static_cast<long double>(xz) - yw) * scale_x);
    matrix[6] = float((static_cast<long double>(yz) + xw) * scale_y);
    matrix[10] = float((1.0L - (static_cast<long double>(ys) + xs)) * scale_z);
    matrix[12] = float(translation_x);
    matrix[13] = float(translation_y);
    matrix[14] = float(translation_z);
    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[15] = 1.0f;
}
} // namespace

void animate_tree_selection(TreeWidget *widget) {
    auto &scene = widget->base.base;
    void *actor = widget->base.actor;
    void *channels = at<void *>(actor, 64);
    const auto first_frame = static_cast<long double>(at<float>(channels, 4));
    const auto last_frame = static_cast<long double>(at<float>(channels, 12));
    if (last_frame > first_frame)
        return;
    void *skeleton = at<void *>(actor, 88);
    char node[260];
    const Dword joint_bits = Dword(widget->selection) + 1u;
    int joint;
    std::memcpy(&joint, &joint_bits, sizeof joint);
    std::sprintf(node, "Root.joint%d", joint);
    if (!game::world::vehicle_render::skeleton_has_node(skeleton, node))
        return;
    const Dword index = game::world::vehicle_render::pending::skeleton_node_index(skeleton, node);
    const float next_angle = float(static_cast<long double>(widget->rotation) + 4.0f);
    auto *pose = reinterpret_cast<float *>(game::world::vehicle_render::animation_pose_scratch()) +
                 index * 12u;
    const bool entering = scene.drag_flags[0] != 0;
    widget->rotation = next_angle;
    const float step = scene.zoom_step;
    scene.drag_flags[1] = 0;
    auto retain = [](float value) { return static_cast<long double>(value); };
    if (entering) {
        const auto inverse = 1.0L / step;
        const auto angle = 540.0L * inverse + next_angle;
        widget->rotation = float(angle);
        if (truncate_tree_value(angle) % 540 == 0)
            widget->rotation = 180;
        const auto target_x = retain(pose[8]) + 0.15f;
        const float moved_x = float(inverse * target_x + retain(scene.camera_origin[0]));
        scene.camera_origin[0] = moved_x;
        const bool keep_x = pose[8] > 0 ? !(target_x < retain(moved_x)) : !(retain(moved_x) < target_x);
        scene.camera_origin[0] = float(keep_x ? retain(moved_x) : target_x);
        const auto moved_y = inverse * pose[9] + scene.camera_origin[1];
        scene.camera_origin[1] = float(moved_y);
        const bool clamp_y = pose[9] > 0 ? retain(pose[9]) < moved_y : moved_y < retain(pose[9]);
        scene.camera_origin[1] = float(clamp_y ? retain(pose[9]) : moved_y);
        const auto delta_z = retain(scene.position.z) - widget->camera_depth;
        auto moved_z = delta_z * inverse + scene.camera_origin[2];
        scene.camera_origin[2] = float(moved_z);
        const auto target_z = retain(scene.position.z) - widget->camera_depth;
        const float stored_target = float(target_z);
        if (target_z < moved_z)
            moved_z = retain(stored_target);
        scene.camera_origin[2] = float(moved_z);
        const auto threshold = (retain(scene.position.z) - widget->camera_depth) - 0.1f;
        if (moved_z >= threshold) {
            widget->rotation = 180;
            scene.drag_flags[1] = 1;
        }
    } else {
        if (scene.camera_origin[2] > 0)
            widget->rotation = float(retain(540) / retain(step) + next_angle);
        const auto inverse = 1.0L / step;
        for (unsigned axis = 0; axis < 2; ++axis) {
            auto moved = retain(scene.camera_origin[axis]) - inverse * pose[8 + axis];
            scene.camera_origin[axis] = float(moved);
            if (pose[8 + axis] > 0 ? moved < retain(0) : retain(0) < moved)
                moved = 0.0L;
            scene.camera_origin[axis] = float(moved);
        }
        auto moved =
            retain(scene.camera_origin[2]) - (retain(scene.position.z) - widget->camera_depth) * inverse;
        scene.camera_origin[2] = float(moved);
        if (moved < retain(0))
            moved = retain(0);
        scene.camera_origin[2] = float(moved);
        auto difference = moved - pose[10];
        if (difference < retain(0))
            difference = -difference;
        if (difference <= retain(0.01f) && scene.camera_origin[0] <= 0 &&
            scene.camera_origin[1] <= 0) {
            std::memcpy(&scene.camera_origin[2], pose + 10, sizeof(Dword));
            scene.drag_flags[1] = 1;
        }
    }
    float rotation[16], transform[16], combined[16], quaternion[4], translation[4];
    tree_rotation_matrix(rotation, widget->rotation);
    tree_pose_matrix(transform, pose, scene, entering);
    graphics::math::multiply(rotation, transform, combined);
    std::memcpy(transform, combined, 64);
    game::world::actor_resources::animation_runtime::decompose_animation_matrix(
        transform, quaternion, translation);
    for (unsigned component = 0; component < 4; ++component)
        std::memcpy(pose + 4 + component, quaternion + component, sizeof(Dword));
    for (unsigned component = 0; component < 4; ++component)
        std::memcpy(pose + 8 + component, translation + component, sizeof(Dword));
}

void begin_tree_exit(TreeWidget *widget) {
    if (void *actor = widget->base.actor) {
        const auto current_frame = static_cast<long double>(widget->base.frame);
        const auto last = static_cast<long double>(at<float>(at<void *>(actor, 64), 12));
        if (last < current_frame) {
            const auto reloaded_last = static_cast<long double>(at<float>(at<void *>(actor, 64), 12));
            const int frame = truncate_tree_value(reloaded_last);
            const float stored_frame = float(static_cast<long double>(frame));
            Dword frame_bits;
            std::memcpy(&frame_bits, &stored_frame, sizeof frame_bits);
            widget->base.frame = stored_frame;
            game::world::actor_resources::set_channel_offset(game::runtime::bytes(actor) + 52, 0,
                                                             frame_bits);
        }
    }
    widget->base.frame_step = float(static_cast<long double>(widget->base.frame_step) * -1.0f);
}
} // namespace nfs::application::frontend
