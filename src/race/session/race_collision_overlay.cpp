#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "race_frame_draw.hpp"
#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::game::world::vehicle_render {
kernel::numeric::Extended body_surface_brightness(void *);
}
namespace nfs::game::world::frame {
using kernel::numeric::Extended;
CollisionOverlay collision_overlay;
static Dword collision_render_state[17];
static Byte collision_state_guard;

CollisionOverlay *construct_collision_overlay(CollisionOverlay *overlay) {
    overlay->enabled = overlay->materials = overlay->brightness = 0;
    overlay->edges = 1;
    overlay->scale = .05f;
    return overlay;
}
// The complete destructor is RET.
void destroy_collision_overlay(CollisionOverlay *) {
}
// Complete saved static shutdown is RET.
static void shutdown_collision_overlay() {
}
// The saved static constructor inlines the first eight bytes.
static void initialize_collision_overlay() {
    construct_collision_overlay(&collision_overlay);
    std::atexit(shutdown_collision_overlay);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_collision_overlay, ".CRT$XCU_NFS_00690028",
                       initialize_collision_overlay);
}

static void shutdown_collision_render_state() {
    graphics::geoprim::destroy(collision_render_state);
}
 
static void bind_collision_render_state() {
    if (!(collision_state_guard & 1)) {
        collision_state_guard |= 1;
        lifecycle::construction::render_state(collision_render_state);
        std::atexit(shutdown_collision_render_state);
    }
    using namespace graphics::geoprim;
    shading(collision_render_state, 1);
    texture_enable(collision_render_state, 0);
    alpha_test(collision_render_state, 0);
    alpha_blend_mode(collision_render_state, 1);
    transparency(collision_render_state, 1);
    culling(collision_render_state, 0);
    depth_method(collision_render_state, 8);
    graphics::immediate::set_gouraud_state(graphics::bindings::single_draw, collision_render_state);
}
static Dword packed_colour(const Byte *argb) {
    return (Dword(argb[0]) << 24) | (Dword(argb[1]) << 16) |
           (Dword(argb[2]) << 8) | Dword(argb[3]);
}

static void draw_collision_edge(CollisionOverlay *overlay, const float *first,
                                const float *second, const Byte *argb) {
    const auto format = kernel::numeric::current_format();
    const float points[2][4] = {
        {float((Extended(first[0], format) - overlay->origin_x) * overlay->scale),
         float((Extended(first[2], format) - overlay->origin_z) * overlay->scale), overlay->depth, 1},
        {float((Extended(second[0], format) - overlay->origin_x) * overlay->scale),
         float((Extended(second[2], format) - overlay->origin_z) * overlay->scale), overlay->depth, 1}};
    const Dword colour = packed_colour(argb);
    graphics::immediate::set_primitive(graphics::bindings::single_draw, 2);
    graphics::immediate::vertex(graphics::bindings::single_draw, points[0], &colour);
    graphics::immediate::vertex(graphics::bindings::single_draw, points[1], &colour);
    graphics::immediate::end_primitive(graphics::bindings::single_draw);
    graphics::immediate::flush(graphics::bindings::single_draw);
}

static void draw_collision_quad(CollisionOverlay *overlay, const float *first,
                                const float *second, const float *third, const float *fourth,
                                const Byte *first_colour, const Byte *second_colour,
                                const Byte *third_colour, const Byte *fourth_colour) {
    const auto format = kernel::numeric::current_format();
    const float *positions[] = {first, second, third, fourth};
    const Byte *argb[] = {first_colour, second_colour, third_colour, fourth_colour};
    float points[4][4];
    Dword colours[4];
    for (unsigned i = 0; i < 4; ++i) {
        points[i][0] = float((Extended(positions[i][0], format) - overlay->origin_x) * overlay->scale);
        points[i][1] = float((Extended(positions[i][2], format) - overlay->origin_z) * overlay->scale);
        points[i][2] = overlay->depth;
        points[i][3] = 1;
        colours[i] = packed_colour(argb[i]);
    }
    graphics::immediate::set_primitive(graphics::bindings::single_draw, Dword(-1));
    for (unsigned i = 0; i < 4; ++i)
        graphics::immediate::vertex(graphics::bindings::single_draw, points[i], &colours[i]);
    graphics::immediate::end_primitive(graphics::bindings::single_draw);
    graphics::immediate::flush(graphics::bindings::single_draw);
}

static void draw_collision_body(CollisionOverlay *overlay, void *body) {
    if (void *surface = at<void *>(body, 532)) {
        const Dword material = at<Byte>(surface, 59) & 15;

        (void)material;
        runtime::particles::lifecycle::pending::diagnostic_message("overpass = %d");
    }
    float *shape = runtime::physics::pending::world_shape(body);
    const Byte red[] = {255, 255, 0, 0};
    Byte last[] = {255, 0, 255, 0};
    if (overlay->brightness) {
        const Byte brightness = Byte(vehicle_render::body_surface_brightness(body).truncate_to_integer());
        draw_collision_quad(overlay, shape, shape + 3, shape + 6, shape + 9,
                            red, red, last, last);
        last[1] = last[2] = last[3] = brightness;
    }
    draw_collision_quad(overlay, shape + 54, shape + 54, shape + 6, shape + 9,
                        red, red, last, last);
}

static constexpr Byte edge_colours[2][4] = {{255, 0, 200, 0}, {255, 0, 0, 200}};
static constexpr Byte polygon_colours[2][4] = {{63, 255, 0, 0}, {63, 0, 0, 255}};
static constexpr Byte material_colours[5][4] = {
    {63, 255, 255, 255}, {63, 255, 0, 0}, {63, 0, 255, 0},
    {63, 0, 0, 255}, {63, 255, 0, 255}};

static void draw_collision_grid(CollisionOverlay *overlay) {
    auto *grid = static_cast<Dword *>(runtime::track::boundary(static_cast<Dword *>(subsystem(36)), 1, 0));
    for (int index = 0; index < at<int>(at<void *>(grid, 12), 0); ++index) {
        void *polygon = runtime::physics::contact::polygon_at(grid, std::uint16_t(index));
        float *corners[4];
        runtime::physics::contact::polygon_corners(polygon, corners);
        int edges = 4;
        // TEST AH,44 / JP leaves four edges for unequal or unordered coordinates.
        if (corners[2][0] == corners[3][0] && corners[2][1] == corners[3][1] &&
            corners[2][2] == corners[3][2])
            edges = 3;
        auto fill = [&](const Byte *colour) {
            draw_collision_quad(overlay, corners[0], corners[1], corners[2], corners[3],
                                colour, colour, colour, colour);
        };
        if (overlay->materials) {
            const unsigned material = at<Byte>(polygon, 59) & 15;
            fill(material_colours[material < 4 ? material : 4]);
        }
        if (overlay->edges) {
            for (int edge = 0; edge < edges; ++edge) {
                const auto kind = at<std::int8_t>(polygon, 60 + edge);
                if (kind == -1 || kind == 1)
                    draw_collision_edge(overlay, corners[edge], corners[(edge + 1) & 3],
                                        edge_colours[kind == 1]);
            }
            bool boundary = false, marked = false;
            for (unsigned edge = 0; edge < 4; ++edge) {
                const auto kind = at<std::int8_t>(polygon, 60 + edge);
                boundary |= kind == -1;
                marked |= kind != 0;
            }
            if (boundary || marked)
                fill(polygon_colours[!boundary]);
        }
        if (overlay->brightness) {
            Byte colours[4][4];
            for (unsigned i = 0; i < 4; ++i) {
                colours[i][0] = 255;
                colours[i][1] = colours[i][2] = colours[i][3] = at<Byte>(polygon, 48 + i);
            }
            draw_collision_quad(overlay, corners[0], corners[1], corners[2], corners[3],
                                colours[0], colours[1], colours[2], colours[3]);
        }
    }
}

void draw_collision_overlay(CollisionOverlay *overlay) {
    if (!overlay->enabled)
        return;
    bind_collision_render_state();
    auto *pose = invoke<Dword *>(tick_services[0], 2);
    float orientation[9];
    runtime::race_audio::camera_orientation(pose, orientation);
    const float matrix[16] = {
        orientation[0], orientation[1], -orientation[2], 0,
        orientation[3], orientation[4], -orientation[5], 0,
        -orientation[6], -orientation[7], orientation[8], 0,
        at<float>(pose, 4), at<float>(pose, 8), -at<float>(pose, 12), 1};
    std::memcpy(offset(graphics::bindings::single_draw, 84), matrix, sizeof matrix);
    void *body = at<void *>(runtime::physics::local_car(static_cast<Dword *>(subsystem(68)), 0), 1768);
    if (body) {
        overlay->origin_x = at<float>(body, 680);
        overlay->origin_z = at<float>(body, 688);
        overlay->depth = -20;
        draw_collision_grid(overlay);
        draw_collision_body(overlay, body);
    }
}
} // namespace nfs::game::world::frame
