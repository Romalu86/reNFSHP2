#include "animation_channel_values.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {

void decompose_animation_matrix(const float *matrix, float *rotation, float *translation) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto root = [&](const Extended &value) {
        return Extended::from_double(std::sqrt(double(value)), format);
    };
    float diagonal_sum = float(X(matrix[10]) + matrix[5]);
    auto trace = X(matrix[10]) + matrix[5] + matrix[0];
    if (X(0) < trace) {
        auto value = root(trace + 1.0f);
        rotation[3] = float(value * .5f);
        auto factor = X(.5f) / value;
        rotation[0] = float((X(matrix[6]) - matrix[9]) * factor);
        rotation[1] = float((X(matrix[8]) - matrix[2]) * factor);
        rotation[2] = float((X(matrix[1]) - matrix[4]) * factor);
    } else {
        unsigned largest = matrix[0] < matrix[5] ? 1 : 0;
        if (matrix[largest * 5] < matrix[10])
            largest = 2;
        Extended value(0, format);
        if (largest == 0)
            value = root(X(matrix[0]) - diagonal_sum + 1.0f);
        else if (largest == 1)
            value = root(X(matrix[5]) - (X(matrix[10]) + matrix[0]) + 1.0f);
        else
            value = root(X(matrix[10]) - (X(matrix[5]) + matrix[0]) + 1.0f);
        rotation[largest] = float(value * .5f);
        auto factor = value;
        if (std::isnan(float(value)) || value < X(0) || X(0) < value)
            factor = X(.5f) / value;
        if (largest == 0) {
            rotation[3] = float((X(matrix[6]) - matrix[9]) * factor);
            rotation[1] = float((X(matrix[4]) + matrix[1]) * factor);
            rotation[2] = float((X(matrix[8]) + matrix[2]) * factor);
        } else if (largest == 1) {
            rotation[3] = float((X(matrix[8]) - matrix[2]) * factor);
            rotation[2] = float((X(matrix[9]) + matrix[6]) * factor);
            rotation[0] = float((X(matrix[4]) + matrix[1]) * factor);
        } else {
            rotation[3] = float((X(matrix[1]) - matrix[4]) * factor);
            rotation[0] = float((X(matrix[8]) + matrix[2]) * factor);
            rotation[1] = float((X(matrix[9]) + matrix[6]) * factor);
        }
    }
    std::memcpy(translation, matrix + 12, 16);
}

void blend_animation_translation(float fraction, const float *first, const float *second,
                                 float *output) {
    auto format = kernel::numeric::current_format();
    for (unsigned i = 0; i < 3; ++i)
        output[i] = float((Extended(second[i], format) - first[i]) * fraction + first[i]);
}
namespace {
bool selected_bone(const Dword *mask, unsigned bone) {
    return !mask || (mask[bone >> 5] & (Dword(1) << (bone & 31))) != 0;
}
void transform_special_bone(void *owner, float *pose) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float scale[3] = {pose[0], pose[1], pose[2]};
    float x = pose[4], y = pose[5], z = pose[6], w = pose[7];
    float xx2 = float(X(x) + x), yy2 = float(X(y) + y), zz2 = float(X(z) + z);
    float xw2 = float(X(xx2) * w), yw2 = float(X(yy2) * w), zw2 = float(X(zz2) * w);
    float xx = float(X(xx2) * x), yx = float(X(yy2) * x), zx = float(X(zz2) * x);
    float yy = float(X(yy2) * y), zy = float(X(zz2) * y);
    auto zz = X(zz2) * z;
    float matrix[16];
    matrix[0] = float((X(1) - (X(yy) + zz)) * scale[0]);
    matrix[4] = float((X(yx) - zw2) * scale[1]);
    matrix[8] = float((X(zx) + yw2) * scale[2]);
    matrix[1] = float((X(yx) + zw2) * scale[0]);
    matrix[5] = float((X(1) - (zz + xx)) * scale[1]);
    matrix[9] = float((X(zy) - xw2) * scale[2]);
    matrix[2] = float((X(zx) - yw2) * scale[0]);
    matrix[6] = float((X(zy) + xw2) * scale[1]);
    matrix[10] = float((X(1) - (X(yy) + xx)) * scale[2]);
    matrix[3] = matrix[7] = matrix[11] = 0;
    matrix[12] = pose[8];
    matrix[13] = pose[9];
    matrix[14] = pose[10];
    matrix[15] = 1;
    graphics::math::append(matrix, reinterpret_cast<const float *>(offset(owner, 48)));
    float rotation[4], translation[4];
    decompose_animation_matrix(matrix, rotation, translation);
    std::memcpy(pose + 4, rotation, 16);
    std::memcpy(pose + 8, translation, 12);
}
void blend_poses(void *owner, float fraction, float *output, const Dword *mask) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    int special = at<int>(owner, 40);
    int bones = at<int>(at<void *>(owner, 12), 8);
    for (int bone = bones - 1; bone >= 0; --bone) {
        if (!selected_bone(mask, unsigned(bone)))
            continue;
        if (special == bone) {
            transform_special_bone(owner, at<float *>(owner, 20) + bone * 12);

            blend_animation_translation(fraction, at<float *>(owner, 16) + special + 8,
                                        at<float *>(owner, 20) + special + 8, output + special + 8);
        }
        auto first = at<const float *>(owner, 16) + bone * 12 + 4;
        auto second = at<const float *>(owner, 20) + bone * 12 + 4;
        auto destination = output + bone * 12 + 4;
        auto dot = X(first[0]) * second[0];
        if (!mask && special >= 0)
            dot = dot + X(second[2]) * first[2] + X(second[1]) * first[1] + X(second[3]) * first[3];
        else if (!mask)
            dot = X(second[2]) * first[2] + X(second[1]) * first[1] + dot + X(second[3]) * first[3];
        else if (special < 0)
            dot = dot + X(second[1]) * first[1] + X(second[2]) * first[2] + X(second[3]) * first[3];
        else
            dot = X(second[3]) * first[3] + X(second[1]) * first[1] + X(second[2]) * first[2] + dot;
        bool opposite = !(X(0) < dot); // C0|C3 test includes unordered.
        for (unsigned axis = 0; axis < 4; ++axis)
            destination[axis] =
                opposite ? float(X(first[axis]) - (X(second[axis]) + first[axis]) * fraction)
                         : float((X(second[axis]) - first[axis]) * fraction + first[axis]);
        auto square = X(destination[0]) * destination[0] + X(destination[3]) * destination[3] +
                      X(destination[1]) * destination[1] + X(destination[2]) * destination[2];
        auto inverse = X(1) / Extended::from_double(std::sqrt(double(square)), format);
        for (unsigned axis = 0; axis < 4; ++axis)
            destination[axis] = float(inverse * destination[axis]);
    }
    if (special < 0 && selected_bone(mask, 0))
        blend_animation_translation(fraction, at<const float *>(owner, 16) + 8,
                                    at<const float *>(owner, 20) + 8, output + 8);
}
float shifted_time(float time, void *owner, unsigned offset_) {
    return float(Extended(time, kernel::numeric::current_format()) - at<float>(owner, offset_));
}
void reset_blend_scratch(void *owner, const Dword *mask) {
    if (at<Byte>(owner, 124))
        vehicle_render::restore_skeleton_pose(at<void *>(owner, 12), at<Dword *>(owner, 16), mask);
    if (at<Byte>(owner, 125))
        vehicle_render::restore_skeleton_pose(at<void *>(owner, 12), at<Dword *>(owner, 20), mask);
}
} // namespace

Dword __fastcall sample_blended_pose(void *owner, void *, Dword time_word, Dword output_word,
                                    Dword mask_word) {
    float time = channel_float(time_word);
    auto output = reinterpret_cast<float *>(output_word);
    auto mask = reinterpret_cast<const Dword *>(mask_word);
    if (!std::isnan(time) && time <= at<float>(owner, 112))
        return Byte(world::invoke<Dword>(at<void *>(owner, 24), 6, shifted_time(time, owner, 32),
                                        output, mask))
                   ? 1
                   : 0;
    if (!(time < at<float>(owner, 120)) && !std::isnan(time) &&
        !std::isnan(at<float>(owner, 120))) {
        if (!Byte(world::invoke<Dword>(at<void *>(owner, 28), 6, shifted_time(time, owner, 36),
                                      output, mask)))
            return 0;
        int special = at<int>(owner, 40);
        if (special >= 0 && selected_bone(mask, unsigned(special)))
            transform_special_bone(owner, output + special * 12);
        return 1;
    }
    auto format = kernel::numeric::current_format();
    float fraction = float((Extended(time, format) - at<float>(owner, 112)) /
                           Extended(at<float>(owner, 116), format));
    reset_blend_scratch(owner, mask);
    if (!Byte(world::invoke<Dword>(at<void *>(owner, 24), 6, shifted_time(time, owner, 32),
                                  at<void *>(owner, 16), mask)))
        return 0;
    if (!Byte(world::invoke<Dword>(at<void *>(owner, 28), 6, shifted_time(time, owner, 36),
                                  at<void *>(owner, 20), mask)))
        return 0;
    blend_poses(owner, fraction, output, mask);
    return 1;
}

void __fastcall advance_blended_animation(void *owner, void *, Dword previous_word,
                                          Dword current_word, Dword output_word) {
    float previous = channel_float(previous_word), current = channel_float(current_word);
    auto output = reinterpret_cast<float *>(output_word);
    if (!std::isnan(current) && current <= at<float>(owner, 112)) {
        world::invoke(at<void *>(owner, 24), 3, shifted_time(previous, owner, 32),
                      shifted_time(current, owner, 32), output);
    } else if (!(current < at<float>(owner, 120)) && !std::isnan(current) &&
               !std::isnan(at<float>(owner, 120))) {
        world::invoke(at<void *>(owner, 28), 3, shifted_time(previous, owner, 36),
                      shifted_time(current, owner, 36), output);
        int special = at<int>(owner, 40);
        if (special >= 0)
            transform_special_bone(owner, output + special * 12);
    } else {
        auto format = kernel::numeric::current_format();
        float fraction = float((Extended(current, format) - at<float>(owner, 112)) /
                               Extended(at<float>(owner, 116), format));
        reset_blend_scratch(owner, nullptr);
        world::invoke(at<void *>(owner, 24), 3, shifted_time(previous, owner, 32),
                      shifted_time(current, owner, 32), at<void *>(owner, 16));
        world::invoke(at<void *>(owner, 28), 3, shifted_time(previous, owner, 36),
                      shifted_time(current, owner, 36), at<void *>(owner, 20));
        blend_poses(owner, fraction, output, nullptr);
    }
}
} // namespace nfs::game::world::actor_resources::animation_runtime
