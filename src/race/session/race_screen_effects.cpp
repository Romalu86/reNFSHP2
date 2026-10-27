#include "race_legacy_effects.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::world::frame {

void draw_screen_effects(void *director) {
    invoke<void *>(director, 2);
    const Dword player = at<Dword>(director, 304) != 0;
    ScreenEffect *head = nullptr;
    auto *effects = reinterpret_cast<ScreenEffect *>(runtime::particles::lifecycle::state_address(7156896 + player * 1800));
    for (unsigned index = 0; index < 50; ++index)
        if (effects[index].remaining) {
            effects[index].next = head;
            head = &effects[index];
        }
    if (!head)
        return;
    std::memcpy(graphics::bindings::single_draw_texture + 84 / 4,
                runtime::particles::lifecycle::state_address(7216296), 64);
    auto E = [](float value) { return static_cast<long double>(value); };
    while (head) {
        if (gui::engine::images::suspend_drawing())
            return;
        void *definition = head->definition;
        float width, height;
        graphics::backend::get_dimensions(graphics::context, &width, &height);
        Dword *state = at<Dword *>(definition, 124);
        const Dword texture = at<Dword>(at<void *>(definition, 144), 8);
        graphics::immediate::set_texture(graphics::bindings::single_draw_texture, texture);
        graphics::immediate::set_render_state(graphics::bindings::single_draw_texture, state);
        graphics::immediate::set_textured_primitive(graphics::bindings::single_draw_texture, 4);
        do {
            if (at<Dword>(at<void *>(head->definition, 144), 8) != texture ||
                at<Dword *>(head->definition, 124) != state)
                break;
            const float size = head->size;
            const float sine_size = float(runtime::turn_sine(head->angle) * size);
            const auto cosine_size = runtime::turn_cosine(head->angle) * size;
            const float x = head->x, y = head->y;
            float corners[4][4];
            corners[0][0] = float((E(x) - sine_size + 1.f) * width * 0.5f);
            corners[0][1] = float((E(0.5f) - (E(y) + cosine_size) * 0.5f) * height);
            corners[1][0] = float((E(x) + cosine_size + 1.f) * width * 0.5f);
            corners[1][1] = float((E(0.5f) - (E(y) + sine_size) * 0.5f) * height);
            corners[2][0] = float((E(x) + sine_size + 1.f) * width * 0.5f);
            corners[2][1] = float((E(0.5f) - (E(y) - cosine_size) * 0.5f) * height);
            corners[3][0] = float((E(x) - cosine_size + 1.f) * width * 0.5f);
            corners[3][1] = float((E(0.5f) - (E(y) - sine_size) * 0.5f) * height);
            for (auto &corner : corners) {
                corner[2] = -1;
                corner[3] = 1;
            }
            constexpr unsigned order[] = {0, 1, 2, 2, 3, 0};
            for (unsigned vertex = 0; vertex < 6; ++vertex)
                graphics::immediate::textured_vertex(graphics::bindings::single_draw_texture,
                                                     corners[order[vertex]], &head->colour,
                                                     effect_uv[vertex]);
            head = head->next;
        } while (head);
        graphics::immediate::end_textured_primitive(graphics::bindings::single_draw_texture);
        graphics::immediate::flush_textured(graphics::bindings::single_draw_texture);
    }
}
} // namespace nfs::game::world::frame
