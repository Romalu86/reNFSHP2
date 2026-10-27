#include "race_frame_draw.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cstring>
namespace nfs::graphics::immediate {
void *draw_array_vertices(Dword *);
}
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::world::frame {
using kernel::numeric::Extended;

static void *first_skid_piece(void *strip) {
    return at<void *>(strip, 0);
}
struct SkidVertex {
    float position[3];
    Dword colour;
    float u, v;
};
static_assert(sizeof(SkidVertex) == 24);
// The entry consumes nine stack arguments.
static Byte append_skid_quad(void *owner, const float *first, const float *second,
                              const float *third, const float *fourth, float first_distance,
                              float last_distance, Dword material, float first_opacity,
                              float last_opacity) {
    const int count = at<int>(owner, 88);
    if (count > 1994)
        return 0;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    if (first_opacity < 0)
        first_opacity = 0;
    else if (first_opacity > 1)
        first_opacity = 1;
    auto last_alpha = E(last_opacity);
    if (last_alpha < E(0))
        last_alpha = E(0);
    else if (E(1) < last_alpha)
        last_alpha = E(1);
    const auto inverse_distance = E(1) / E(50);
    float first_alpha = float((E(1) - E(first_distance) * inverse_distance) * 143.f * first_opacity);
    last_alpha = (E(1) - inverse_distance * last_distance) * 143.f * last_alpha;
    if (first_alpha < 0)
        first_alpha = 0;
    else if (first_alpha > 255)
        first_alpha = 255;
    if (last_alpha < E(0))
        last_alpha = E(0);
    else if (E(255) < last_alpha)
        last_alpha = E(255);
    const Dword first_colour = (Dword(E(first_alpha).truncate_to_integer()) << 24) | 16777215;
    const Dword last_colour = (Dword(last_alpha.truncate_to_integer()) << 24) | 16777215;
    auto *vertices = static_cast<SkidVertex *>(graphics::immediate::draw_array_vertices(at<Dword *>(owner, 12)));
    vertices += count;
    const float first_u = at<float>(owner, 100 + material * 4);
    const float last_u = at<float>(owner, 124 + material * 4);
    std::memcpy(vertices[0].position, first, 12);
    vertices[0].u = first_u;
    vertices[0].v = 0;
    vertices[0].colour = last_colour;
    std::memcpy(vertices[1].position, second, 12);
    vertices[1].u = last_u;
    vertices[1].v = 0;
    vertices[1].colour = last_colour;
    std::memcpy(vertices[2].position, third, 12);
    vertices[2].u = last_u;
    vertices[2].v = 1;
    vertices[2].colour = first_colour;
    std::memcpy(vertices[3].position, fourth, 12);
    vertices[3].u = first_u;
    vertices[3].v = 1;
    vertices[3].colour = first_colour;
    vertices[4] = vertices[0];
    vertices[5] = vertices[2];
    at<int>(owner, 88) = count + 6;
    return 1;
}

void draw_skid_traces(void *owner, void *director) {
    if (!at<Byte>(subsystem(140), 12) || at<Byte>(subsystem(48), 104))
        return;
    at<Dword>(owner, 88) = 0;
    graphics::backend::set_depth_write(graphics::context, 0);
    float eye[3];
    camera::active_camera_position(director, eye);
    eye[2] = -eye[2];
    float orientation[9];
    runtime::race_audio::camera_orientation(invoke<Dword *>(director, 2), orientation);
    const float forward[3] = {orientation[6], orientation[7], -orientation[8]};
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    Byte available = 1;
    for (void *strip = at<void *>(owner, 4); strip && available; strip = at<void *>(strip, 76)) {
        void *previous = nullptr;
        for (void *piece = first_skid_piece(strip); piece && available;
             previous = piece, piece = at<void *>(piece, 56)) {
            float corners[4][3];
            std::memcpy(corners, piece, sizeof corners);
            float first_delta[3], last_delta[3];
            for (unsigned axis = 0; axis < 3; ++axis) {
                first_delta[axis] = float(E(corners[0][axis]) - eye[axis]);
                last_delta[axis] = float(E(corners[3][axis]) - eye[axis]);
            }
            const auto first_depth = runtime::retained_dot_product(first_delta, forward);
            if (!(E(0) < first_depth)) {
                const auto last_depth = runtime::retained_dot_product(last_delta, forward);
                if (!(E(0) < last_depth))
                    continue;
            }
            float squared = float(runtime::retained_dot_product(first_delta, first_delta));
            const float first_distance = float(E(squared).square_root());
            squared = float(runtime::retained_dot_product(last_delta, last_delta));
            const float last_distance = float(E(squared).square_root());
            if (!(first_distance < 50) && !(last_distance < 50))
                continue;
            const float opacity = at<float>(piece, 48);
            const float previous_opacity = previous ? at<float>(previous, 48) : opacity;
            available = append_skid_quad(owner, corners[0], corners[1], corners[2], corners[3],
                                           first_distance, last_distance, at<Dword>(piece, 52),
                                           previous_opacity, opacity);
        }
    }
    if (at<int>(owner, 88) > 0)
        graphics::immediate::draw_array(at<Dword *>(owner, 12), at<int>(owner, 88));
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::world::frame
