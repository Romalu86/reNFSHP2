#include "animation_channel_values.hpp"
#include <initializer_list>
namespace nfs::game::world::actor_resources::animation_runtime {

void mirror_animation_pose(void *skeleton, void *source, void *destination, Byte retain_root,
                           const Dword *mask) {
    auto flip = [](Dword word) { return word ^ 2147483648u; };
    int count = at<int>(skeleton, 8);
    if (source == destination) {
        for (int index = 0; index < count; ++index) {
            if (mask && !(mask[Dword(index) >> 5] & (1u << (Dword(index) & 31))))
                continue;
            int mirrored = at<int>(skeleton, 60 + Dword(index) * 112);
            Dword *first = reinterpret_cast<Dword *>(offset(destination, Dword(index) * 48));
            if (index < mirrored) {
                Dword *second = reinterpret_cast<Dword *>(offset(destination, Dword(mirrored) * 48));
                for (unsigned component : {4u, 5u, 6u, 7u, 8u, 9u, 10u}) {
                    Dword previous = first[component], next = second[component];
                    bool negative = component == 4 || component == 5 || component == 10;
                    second[component] = negative ? flip(previous) : previous;
                    first[component] = negative ? flip(next) : next;
                }
            } else if (index == mirrored) {
                first[4] = flip(first[4]);
                first[5] = flip(first[5]);
                first[10] = flip(first[10]);
            }
        }
    } else {
        for (int index = 0; index < count; ++index) {
            if (mask && !(mask[Dword(index) >> 5] & (1u << (Dword(index) & 31))))
                continue;
            int mirrored = at<int>(skeleton, 60 + Dword(index) * 112);
            Dword *input = reinterpret_cast<Dword *>(offset(source, Dword(index) * 48));
            Dword *output = reinterpret_cast<Dword *>(offset(destination, Dword(mirrored) * 48));
            output[0] = input[0];
            output[1] = input[1];
            output[2] = input[2];
            output[4] = flip(input[4]);
            output[5] = flip(input[5]);
            output[6] = input[6];
            output[7] = input[7];
            output[8] = input[8];
            output[9] = input[9];
            output[10] = flip(input[10]);
        }
    }
    if (!retain_root) {
        auto *root = static_cast<Dword *>(destination);
        Dword old_x = root[4], old_w = root[7];
        root[4] = root[6];
        root[6] = flip(old_x);
        root[7] = flip(root[5]);
        root[8] = flip(root[8]);
        root[10] = flip(root[10]);
        root[5] = old_w;
    }
}
} // namespace nfs::game::world::actor_resources::animation_runtime
