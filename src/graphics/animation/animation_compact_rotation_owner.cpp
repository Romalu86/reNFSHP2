#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {

Byte *rotation_block(void *resource, Dword index) {
    Dword count = at<Byte>(resource, 6);
    Dword length = Dword(1) << (at<Byte>(resource, 16) & 31);
    return offset(resource, count * 16 + 18 + (((length + 1) * count * 3 + 1) & ~Dword(1)) * index);
}

Byte *rotation_block_tail(void *resource) {
    Dword length = Dword(1) << (at<Byte>(resource, 16) & 31);
    Dword count = at<Byte>(resource, 6), frames = at<std::uint16_t>(resource, 4);
    Dword remainder = frames % length;
    auto tail = rotation_block(resource, frames / length);
    if (int(remainder) > 0)
        tail += (remainder + 1) * count * 3;
    return tail;
}

void __fastcall bind_compact_rotation_animation(void *owner, void *, Dword resource_word) {
    auto resource = reinterpret_cast<void *>(resource_word);
    at<void *>(owner, 12) = resource;
    at<void *>(owner, 20) = offset(resource, Dword(at<Byte>(resource, 6)) * 16 + 18);
    at<void *>(owner, 52) = rotation_block_tail(resource);
    at<Dword>(owner, 56) =
        (Dword(rotation_block_tail(resource)) + 1 + at<Byte>(resource, 7)) & ~Dword(1);
    Dword length = Dword(1) << (at<Byte>(resource, 16) & 31);
    at<Dword>(owner, 24) = ((length + 1) * Dword(at<Byte>(resource, 6)) * 3 + 1) & ~Dword(1);
    if (!at<Byte>(resource, 6))
        return;
    at<void *>(owner, 32) = at<void *>(owner, 36) =
        allocate_animation_buffer(Dword(at<Byte>(resource, 6)) << 4);
    at<void *>(owner, 44) = at<void *>(owner, 48) =
        allocate_animation_buffer(Dword(at<Byte>(resource, 6)) << 4);
    at<void *>(owner, 16) = allocate_animation_buffer(Dword(at<Byte>(resource, 6)) << 5);
    auto format = kernel::numeric::current_format();
    for (Dword i = 0; i < at<Byte>(resource, 6); ++i) {
        auto encoded = reinterpret_cast<const std::uint16_t *>(offset(resource, 18 + i * 16));
        auto coefficients = at<float *>(owner, 16) + i * 8;
        for (unsigned j = 0; j < 8; ++j) {
            auto value = Extended::integer(encoded[j], format) * 0.0000305180437862873077392578125f;
            coefficients[j] = float(j < 4 ? value - 1.0f : value);
        }
    }
}
} // namespace nfs::game::world::actor_resources::animation_runtime
