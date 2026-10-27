#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
struct RootComponents {
    float x;
    Extended y, z, w;
};
RootComponents blended_components(const float *first, const float *second, float weight,
                                  bool duration) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto dot = X(second[3]) * first[3] + X(second[2]) * first[2] + X(second[1]) * first[1] +
               X(second[0]) * first[0];
    bool reverse = !(X(0) < dot);
    auto component = [&](unsigned i) {
        return reverse ? X(first[i]) - (X(second[i]) + first[i]) * weight
                       : (X(second[i]) - first[i]) * weight + first[i];
    };
    auto x = component(0), y = component(1);
    float z = float(component(2)), w = float(component(3));
    float inverse = float(X(1) / (X(w) * w + X(z) * z + y * y + x * x).square_root());
    auto normalized_y = y * inverse, normalized_z = X(z) * inverse, normalized_w = X(w) * inverse;
    return {float(x * inverse), normalized_y, duration ? normalized_z : X(float(normalized_z)),
            duration ? X(float(normalized_w)) : normalized_w};
}
void root_direction(const RootComponents &q, float *out, bool duration) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto twice_x = X(q.x) + q.x;
    if (duration)
        twice_x = X(float(twice_x));
    auto twice_y = q.y + q.y;
    float twice_z = float(q.z + q.z);
    float yw = float(q.w * twice_y), zx = float(X(twice_z) * q.x), yy = float(q.y * twice_y);
    auto horizontal = X(1) - (q.z * twice_z + yy) + (X(zx) + yw);
    auto vertical = X(1) - (twice_x * q.x + yy) + (X(zx) - yw);
    out[0] = float((twice_y * q.x - q.w * twice_z) + horizontal * 0.0f);
    out[1] = float(q.w * twice_x + q.y * twice_z + vertical * 0.0f);
}
Dword duration_direction(void *owner, float *out, bool end) {
    float first[4], second[4];
    if (end) {
        clip_end_root_rotation(at<void *>(owner, 16), nullptr, first);
        clip_end_root_rotation(at<void *>(owner, 20), nullptr, second);
    } else {
        clip_start_root_rotation(at<void *>(owner, 16), nullptr, first);
        clip_start_root_rotation(at<void *>(owner, 20), nullptr, second);
    }
    root_direction(blended_components(first, second, at<float>(owner, 24), true), out, true);
    return 1;
}
} // namespace

void __fastcall sample_frame_root_rotation(void *owner, void *, float first_time, float second_time,
                                           float *out) {
    auto scratch = static_cast<float *>(animation_shared_block(0)->address);
    if (!Byte(world::invoke<Dword>(at<void *>(owner, 24), 6, channel_word(first_time), scratch, 0u)))
        return;
    float first[4];
    std::memcpy(first, scratch + 4, 16);
    if (at<float>(owner, 40) != 0) {
        vehicle_render::restore_skeleton_pose(at<void *>(owner, 52),
                                              reinterpret_cast<Dword *>(scratch), nullptr);
        if (!Byte(world::invoke<Dword>(at<void *>(owner, 28), 6, channel_word(second_time), scratch,
                                      0u)))
            return;
        auto format = kernel::numeric::current_format();
        auto X = [&](float v) { return Extended(v, format); };
        auto dot = X(scratch[7]) * first[3] + X(scratch[6]) * first[2] + X(scratch[5]) * first[1] +
                   X(scratch[4]) * first[0];
        float weight = at<float>(owner, 40);
        for (unsigned i = 0; i < 4; ++i)
            out[i] = float(!(X(0) < dot) ? X(first[i]) - (X(scratch[4 + i]) + first[i]) * weight
                                         : (X(scratch[4 + i]) - first[i]) * weight + first[i]);
        auto square =
            X(out[0]) * out[0] + X(out[3]) * out[3] + X(out[2]) * out[2] + X(out[1]) * out[1];
        auto inverse = X(1) / square.square_root();
        for (unsigned i = 0; i < 4; ++i)
            out[i] = float(inverse * out[i]);
    } else
        std::memcpy(out, first, 16);
}

void __fastcall clip_start_root_rotation(void *owner, void *, float *out) {
    sample_frame_root_rotation(owner, nullptr, 0, 0, out);
}

void __fastcall clip_end_root_rotation(void *owner, void *, float *out) {
    auto frames = at<void **>(owner, 16);
    int index = at<int>(owner, 48);
    sample_frame_root_rotation(owner, nullptr, float(int(at<std::uint16_t>(frames[index], 4)) - 1),
                               float(int(at<std::uint16_t>(frames[index + 1], 4)) - 1), out);
}

Dword __fastcall duration_start_direction(void *owner, void *, float *out) {
    return duration_direction(owner, out, false);
}

Dword __fastcall duration_end_direction(void *owner, void *, float *out) {
    return duration_direction(owner, out, true);
}

Dword __fastcall sample_frame_direction(void *owner, void *, float first_time, float second_time,
                                       float *out) {
    auto scratch = static_cast<float *>(animation_shared_block(0)->address);
    if (!Byte(world::invoke<Dword>(at<void *>(owner, 24), 6, channel_word(first_time), scratch, 0u)))
        return 0;
    float first[4];
    std::memcpy(first, scratch + 4, 16);
    auto format = kernel::numeric::current_format();
    RootComponents rotation{first[0], Extended(first[1], format), Extended(first[2], format),
                            Extended(first[3], format)};
    if (at<float>(owner, 40) != 0) {
        vehicle_render::restore_skeleton_pose(at<void *>(owner, 52),
                                              reinterpret_cast<Dword *>(scratch), nullptr);
        if (!Byte(world::invoke<Dword>(at<void *>(owner, 28), 6, channel_word(second_time), scratch,
                                      0u)))
            return 0;
        rotation = blended_components(first, scratch + 4, at<float>(owner, 40), false);
    }
    root_direction(rotation, out, false);
    return 1;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
