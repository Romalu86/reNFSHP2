#include "hud_map.hpp"
#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
namespace nfs::graphics::gouraud {
// Missing-stream locals retain their saved indeterminate bytes.
void read_vertex(Dword *owner, Dword index, float *position, Dword *colour) {
    std::byte retained[16];
    if (owner[2768 / 4]) {
        auto *descriptor = game::runtime::bytes(owner) + (owner[2768 / 4] - 1) * 272;
        if (auto *stream = game::runtime::at<std::uint8_t *>(descriptor, 304)) {
            const auto *source = stream + game::runtime::at<Dword>(descriptor, 308) * index;
            std::memcpy(retained, source, sizeof(retained));
        }
    }
    std::memcpy(position, retained, 12);
    std::memcpy(colour, retained + 12, 4);
}
} // namespace nfs::graphics::gouraud
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using kernel::numeric::Extended;

static void *map_section(void *catalogue, int index) {
    return at<void *>(catalogue, 15656 + Dword(index) * 4);
}

void map_position(Map *map, float *output, const float *input) {
    float position[3] = {input[0], input[1], input[2]};
    if (map->half_width != 0 && map->half_height != 0) {
        const auto format = kernel::numeric::current_format();
        position[2] = -1;
        position[0] =
            float((Extended(map->centre_x, format) - input[0]) / Extended(map->half_width, format));
        position[1] = float((Extended(input[2], format) - map->centre_z) /
                            Extended(map->half_height, format));
        if (map->reversed)
            position[0] = -position[0];
    }
    output[0] = position[0];
    output[1] = position[1];
    output[2] = position[2];
}

void build_map_geometry(Map *map) {
    if (map->geometry)
        return;
    const int sections = at<std::uint16_t>(system(76), 16860);
    void *catalogue = game::runtime::bytes(system(76)) + 4;
    Dword vertices = 0;
    map->vertex_count = 0;
    for (int index = 0; index < sections; ++index) {
        void *section = map_section(catalogue, index);
        if (at<int>(section, 32) <= map->section_detail_limit)
            for (int segment = 0; segment < at<int>(section, 36) - 1; ++segment)
                vertices += 6;
    }
    map->vertex_count = vertices;
    if (!vertices)
        return;
    auto *array =
        static_cast<Dword *>(lifecycle::allocate(2776, "HudMap::GouraudArray", map->pool));
    if (array) {
        try {
            array = graphics::gouraud::construct(array, vertices, map->pool, 4);
        } catch (...) {
            game::runtime::track::pending::release_named(
                array, "HudMap::GouraudArray", map->pool);
            throw;
        }
    }
    map->geometry = array;
    struct State {
        Dword fields[17];
        State() {
            lifecycle::construction::render_state(fields);
        }
        ~State() {
            graphics::geoprim::destroy(fields);
        }
    } state;
    namespace gp = graphics::geoprim;
    gp::primitive(state.fields, 4);
    gp::shading(state.fields, 1);
    gp::texture_enable(state.fields, 0);
    gp::transparency(state.fields, 1);
    gp::alpha_blend_mode(state.fields, 1);
    gp::depth_method(state.fields, 8);
    gp::alpha_test(state.fields, 0);
    graphics::gouraud::set_state(array, state.fields);
    float minimum_x = 0, maximum_x = 0, minimum_z = 0, maximum_z = 0;
    Dword vertex = 0;
    const auto format = kernel::numeric::current_format();
    for (int index = 0; index < sections; ++index) {
        void *section = map_section(catalogue, index);
        if (at<int>(section, 32) > map->section_detail_limit)
            continue;
        for (int segment = 0; segment < at<int>(section, 36) - 1; ++segment) {
            float first[21], second[21], corners[4][3];
            game::runtime::ai::dependencies::sample_section_pose(section, segment, first);
            for (int axis = 0; axis < 3; ++axis) {
                corners[0][axis] = float(Extended(first[12 + axis], format) + first[axis]);
                corners[1][axis] = float(Extended(first[15 + axis], format) + first[axis]);
            }
            if (!(minimum_x < first[0]))
                minimum_x = first[0];
            if (!(maximum_x > first[0]))
                maximum_x = first[0];
            if (!(minimum_z < first[2]))
                minimum_z = first[2];
            if (!(maximum_z > first[2]))
                maximum_z = first[2];
            game::runtime::ai::dependencies::sample_section_pose(section, segment + 1, second);
            // The second endpoint's two edges are reversed in the saved triangle order.
            for (int axis = 0; axis < 3; ++axis) {
                corners[2][axis] = float(Extended(second[15 + axis], format) + second[axis]);
                corners[3][axis] = float(Extended(second[12 + axis], format) + second[axis]);
            }
            for (int corner : {0, 1, 2, 0, 2, 3})
                graphics::gouraud::set_vertex(array, vertex++, corners[corner],
                                              &map->spline_colour);
        }
    }
    auto width = Extended(maximum_x, format) - minimum_x;
    if (width < Extended(0, format))
        width = -width;
    map->half_width = float(width * .5f);
    auto height = Extended(maximum_z, format) - minimum_z;
    if (height < Extended(0, format))
        height = -height;
    map->half_height = float(height * .5f);
    map->centre_x = float((Extended(maximum_x, format) + minimum_x) * .5f);
    map->centre_z = float((Extended(maximum_z, format) + minimum_z) * .5f);
    for (int index = 0; index < int(map->vertex_count); ++index) {
        Dword colour = 0;
        float input[3], normalized[3];
        graphics::gouraud::read_vertex(array, Dword(index), input, &colour);
        map_position(map, normalized, input);
        graphics::gouraud::set_vertex(array, Dword(index), normalized, &colour);
    }
    if (map->half_width < map->half_height)
        for (float &heading : map->heading_history)
            heading = .25f;
    graphics::gouraud::set_matrix(array, game::runtime::skids::lifecycle::identity_matrix);
}

void __fastcall initialize_map(Map *map, void *) {
    gui::widgets::pending::request_general_layout(&map->base, nullptr);
    build_map_geometry(map);
}

void clear_map_geometry(Map *map) {
    if (map->geometry) {
        invoke(map->geometry, 0, Dword(1));
        map->geometry = nullptr;
    }
    for (float &sample : map->zoom_history)
        sample = 0;
}

void __fastcall release_map(Map *map, void *) {
    clear_map_geometry(map);
    gui::widgets::clear_layout_request(&map->base.base, nullptr);
}

void set_map_matrix(Map *map, const Dword *matrix) {
    graphics::gouraud::set_matrix(map->geometry, matrix);
}
} // namespace nfs::application::frontend::hud
