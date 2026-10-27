#include "runtime_animation_dispatch.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
#include <limits>
namespace nfs::game::world::dependency::channels {
void read_quaternion(const float **, float *);
void read_euler_rotation(const float **, float *);
void read_translation(const float **, float *);
void interpolate_quaternion(float, const float **, const float **, float *);
void interpolate_euler_rotation(float, const float **, const float **, float *);
void interpolate_translation(float, const float **, const float **, float *);
} // namespace nfs::game::world::dependency::channels
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
float floating(Dword word) {
    float value;
    std::memcpy(&value, &word, 4);
    return value;
}
int animation_frame_index(float time) {
    auto format = kernel::numeric::current_format();
    auto value = kernel::numeric::Extended(time, format);
    // FCOM's unordered condition follows the saved negative arm.
    float adjusted = float(time < 0 || std::isnan(time) ? value + 0.499999f : value - 0.499999f);
    auto integer = kernel::numeric::Extended(adjusted, format).round_to_integer();
    return integer < INT32_MIN || integer > INT32_MAX ? INT32_MIN : int(integer);
}
bool bone_selected(const Dword *mask, unsigned index) {
    return !mask || (mask[index >> 5] & (1u << (index & 31)));
}
} // namespace

void read_uncompressed_frame(void *animation, int frame, void *pose, const Dword *mask) {
    using namespace dependency::channels;
    Dword *signature = reinterpret_cast<Dword *>(offset(animation, 16));
    Dword *end = signature + at<Dword>(animation, 4);
    const float *cursor = reinterpret_cast<float *>(offset(
        animation, (at<Dword>(animation, 8) * Dword(frame) + 4 + at<Dword>(animation, 4) * 2) * 4));
    unsigned bone = 0;
    for (auto *output = reinterpret_cast<float *>(offset(pose, 16)); signature < end;
         output += 12, ++bone) {
        int count = int(*signature++);
        while (count-- > 0) {
            Dword operation = *signature++;
            if (bone_selected(mask, bone))
                reinterpret_cast<void(__cdecl *)(const float **, float *)>(operation)(&cursor,
                                                                                      output);
            else if (operation == reinterpret_cast<Dword>(read_euler_rotation) ||
                     operation == reinterpret_cast<Dword>(read_translation))
                cursor += 3;
            else if (operation == reinterpret_cast<Dword>(read_quaternion))
                cursor += 4;
        }
    }
}

void sample_uncompressed_pose(void *animation, float time, void *pose, Byte interpolate,
                              const Dword *mask) {
    using namespace dependency::channels;
    int frame = animation_frame_index(time);
    if (frame < 0) {
        read_uncompressed_frame(animation, 0, pose, mask);
        return;
    }
    int last = int(at<Dword>(animation, 12) - 1);
    if (frame >= last) {
        read_uncompressed_frame(animation, last, pose, mask);
        return;
    }
    auto format = kernel::numeric::current_format();
    float fraction = float(kernel::numeric::Extended(time, format) -
                           kernel::numeric::Extended::integer(frame, format));
    if (!interpolate || !fraction || std::isnan(fraction)) {
        read_uncompressed_frame(animation, frame, pose, mask);
        return;
    }
    Dword signatures = at<Dword>(animation, 4), stride = at<Dword>(animation, 8);
    Dword *signature = reinterpret_cast<Dword *>(offset(animation, 16 + signatures * 4));
    Dword *end = signature + signatures;
    const float *first = reinterpret_cast<float *>(
        offset(animation, (stride * Dword(frame) + signatures * 2 + 4) * 4));
    const float *second = reinterpret_cast<float *>(
        offset(animation, (stride * (Dword(frame) + 1) + signatures * 2 + 4) * 4));
    unsigned bone = 0;
    for (auto *output = reinterpret_cast<float *>(offset(pose, 16)); signature < end;
         output += 12, ++bone) {
        int count = int(*signature++);
        while (count-- > 0) {
            Dword operation = *signature++;
            if (bone_selected(mask, bone))
                reinterpret_cast<void(__cdecl *)(float, const float **, const float **, float *)>(
                    operation)(fraction, &first, &second, output);
            else if (operation == reinterpret_cast<Dword>(interpolate_euler_rotation) ||
                     operation == reinterpret_cast<Dword>(interpolate_translation)) {
                first += 3;
                second += 3;
            } else if (operation == reinterpret_cast<Dword>(interpolate_quaternion)) {
                first += 4;
                second += 4;
            }
        }
    }
}

void __fastcall advance_format_0(void *receiver, void *, Dword, Dword second_time, Dword output) {
    sample_uncompressed_pose(at<void *>(receiver, 12), floating(second_time),
                             reinterpret_cast<void *>(output), at<Byte>(receiver, 16), nullptr);
}

Dword __fastcall sample_masked_format_0(void *receiver, void *, Dword time, Dword output, Dword mask) {
    sample_uncompressed_pose(at<void *>(receiver, 12), floating(time),
                             reinterpret_cast<void *>(output), at<Byte>(receiver, 16),
                             reinterpret_cast<Dword *>(mask));
    return 1;
}

void __fastcall advance_format_2(void *receiver, void *, Dword, Dword second_time, Dword output) {
    void *resource = at<void *>(receiver, 12);
    float time = floating(second_time);
    int frame = animation_frame_index(time);
    Dword count = at<std::uint16_t>(resource, 4), aligned_count = (count + 1) & ~1u;
    auto *indices = reinterpret_cast<std::uint16_t *>(offset(resource, 8));
    auto *values = reinterpret_cast<Dword *>(offset(resource, 8 + aligned_count * 2));
    if (frame < 0)
        frame = 0;
    else {
        int last = int(at<std::uint16_t>(resource, 6)) - 1;
        if (frame >= last)
            frame = last;
        else {
            float fraction = time - float(frame);
            if (fraction && !std::isnan(fraction) && at<Byte>(receiver, 16)) {
                auto format = kernel::numeric::current_format();
                auto *first = reinterpret_cast<float *>(values + count * Dword(frame));
                auto *second = first + count;
                for (Dword index = 0; index < count; ++index)
                    reinterpret_cast<float *>(output)[indices[index]] =
                        float((kernel::numeric::Extended(second[index], format) - first[index]) *
                                  fraction +
                              first[index]);
                return;
            }
        }
    }
    for (Dword index = 0; index < count; ++index)
        reinterpret_cast<Dword *>(output)[indices[index]] = values[count * Dword(frame) + index];
    return;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
