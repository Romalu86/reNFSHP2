#include "race_legacy_effects.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::game::world::frame {

void draw_world_effects(void *director) {
    if (!at<Byte>(subsystem(140), 20) || gui::engine::images::suspend_drawing() ||
        at<Byte>(subsystem(48), 104))
        return;
    const Dword player = at<Dword>(director, 304) != 0;
    Dword *pose = invoke<Dword *>(director, 2);
    float orientation[9];
    runtime::race_audio::camera_orientation(pose, orientation);
    auto E = [](float value) { return static_cast<long double>(value); };
    auto state = [](unsigned address) { return runtime::particles::lifecycle::state_address(address); };
    if (!player) {
        auto &previous = at<Dword>(state(7156732), 0);
        const int now = std::int32_t(graphics::unresolved::camera_state) >> 1;
        if (!previous)
            previous = Dword(now);
        const auto delta = std::int32_t(Dword(now) - previous);
        if (delta > 0) {
            previous = Dword(now);
            const auto frames = static_cast<long double>(delta) * 0.5f;
            const auto seconds = E(0.01666666753590106964111328125f) * frames;
            for (unsigned index = 0; index < 450; ++index) {
                void *particle = state(7160496 + index * 124);
                if (!at<std::uint16_t>(particle, 4) || !at<void *>(particle, 0))
                    continue;
                void *definition = at<void *>(particle, 0);
                at<float>(particle, 36) = float(frames * at<float>(particle, 44) + at<float>(particle, 36));
                at<float>(particle, 48) = float(frames * at<float>(particle, 52) + at<float>(particle, 48));
                const auto vertical_velocity = frames * at<float>(definition, 52) + at<float>(particle, 28);
                at<float>(particle, 28) = float(vertical_velocity);
                at<float>(particle, 12) = float(seconds * at<float>(particle, 24) + at<float>(particle, 12));
                at<float>(particle, 16) = float(vertical_velocity * seconds + at<float>(particle, 16));
                at<float>(particle, 20) = float(seconds * at<float>(particle, 32) + at<float>(particle, 20));
            }
        }
    }
    sample_frame_rate();
    float eye[3];
    std::memcpy(eye, bytes(pose) + 4, 12);
    void *trail_head = nullptr;
    for (unsigned index = 0; index < 32; ++index)
        at<Dword>(state(7216660 + index * 16), 0) = 0;
    orientation[2] = -orientation[2];
    orientation[5] = -orientation[5];
    orientation[6] = -orientation[6];
    orientation[7] = -orientation[7];
    eye[2] = -eye[2];
    auto *screen = reinterpret_cast<ScreenEffect *>(state(7156896 + player * 1800));
    auto random_fraction = [&] {
        return static_cast<long double>(runtime::emitter_random_word() & 65535) * 0.0000152587890625f;
    };
    for (unsigned index = 0; index < 450; ++index) {
        void *particle = state(7160496 + index * 124);
        if (!at<std::uint16_t>(particle, 4))
            continue;
        void *definition = at<void *>(particle, 0);
        if (!at<void *>(definition, 108) || !at<void *>(definition, 96))
            continue;
        float delta[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            delta[axis] = float(E(at<float>(particle, 12 + axis * 4)) - eye[axis]);
        const auto depth = runtime::hardware::dot_product(delta, orientation + 6);
        at<float>(particle, 116) = float(depth);
        const bool trail = at<float>(definition, 100) != 0;
        const bool retained = trail && at<Byte>(particle, 112 + player) &&
                              at<float>(particle, 96 + player * 12) < 1;
        if (retained || (!(E(0) < depth) && !(depth < -E(at<float>(definition, 104))))) {
            if (at<float>(definition, 100) != 0) {
                at<void *>(particle, 120) = trail_head;
                trail_head = particle;
            } else {
                void *material = at<void *>(definition, 108);
                at<void *>(particle, 120) = at<void *>(material, 12);
                at<void *>(material, 12) = particle;
            }
            continue;
        }
        const int amount = at<int>(definition, 116);
        at<Byte>(particle, 112 + player) = 0;
        if (!amount || !(at<float>(particle, 116) > 0) || !(at<float>(particle, 116) < 2))
            continue;
        const float x = float(runtime::hardware::dot_product(delta, orientation));
        const float y = float(runtime::hardware::dot_product(delta, orientation + 3));
        const float absolute_x = x < 0 ? -x : x, absolute_y = y < 0 ? -y : y;
        if (!(absolute_x < 1) || !(absolute_y < 1) || !at<Byte>(subsystem(140), 13))
            continue;
        const int kind = at<int>(definition, 4);
        if (kind > 0 && kind < 20 &&
            at<int>(state(7156736), (Dword(kind) + player * 20) * 4) > at<int>(definition, 148))
            continue;
        const float probability = at<float>(definition, 140);
        if (probability != 0) {
            const auto random = random_fraction();
            if (E(probability) < random)
                continue;
        }
        for (int emitted = 0; emitted < at<int>(definition, 116); ++emitted) {
            unsigned slot = 0;
            while (slot < 50 && screen[slot].remaining)
                ++slot;
            if (slot == 50)
                break;
            const float scatter = at<float>(definition, 132);
            float screen_x = x, screen_y = y;
            if (scatter != 0) {
                const auto random_x = random_fraction() * scatter;
                screen_x = float(random_x + random_x - scatter + x);
                const auto random_y = random_fraction() * scatter;
                screen_y = float(random_y + random_y - scatter + y);
            }
            bool overlaps = false;
            for (unsigned existing = 0; existing < 50; ++existing) {
                if (!screen[existing].remaining)
                    continue;
                const auto radius = E(screen[existing].size) * 0.5f;
                auto horizontal = E(screen_x) - screen[existing].x;
                if (horizontal < E(0))
                    horizontal = -horizontal;
                if (!(horizontal < radius))
                    continue;
                auto vertical = E(screen_y) - screen[existing].y;
                if (vertical < E(0))
                    vertical = -vertical;
                if (vertical < radius) {
                    overlaps = true;
                    break;
                }
            }
            if (overlaps)
                continue;
            const int lifetime = at<int>(definition, 120);
            const Dword random_lifetime = runtime::emitter_random_word();
            screen[slot].definition = definition;
            screen[slot].remaining = Dword(lifetime) - random_lifetime % Dword(lifetime >> 1);
            const float size = at<float>(definition, 128);
            const Dword random_size = runtime::emitter_random_word();
            screen[slot].angle = float(random_fraction());
            const Dword colour = at<Dword>(definition, 136);
            screen[slot].x = screen_x;
            screen[slot].y = screen_y;
            screen[slot].depth = -1;
            screen[slot].colour = colour;
            const auto variation = static_cast<long double>(random_size & 65535) * 0.0000152587890625f;
            screen[slot].size = float(E(size) - E(size) * 0.5f * variation);
        }
    }
    graphics::immediate::flush_textured(graphics::bindings::single_draw_texture);
    graphics::backend::set_depth_write(graphics::context, 0);
    std::memcpy(graphics::bindings::single_draw_texture + 84 / 4, state(7216296), 64);
    for (unsigned index = 0; index < 32; ++index) {
        void *head = at<void *>(state(7216660 + index * 16), 0);
        while (head)
            head = draw_rotated_effect_batch(head, orientation);
    }
    while (trail_head)
        trail_head = draw_trail_effect_batch(trail_head, orientation, eye, player);
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::world::frame
