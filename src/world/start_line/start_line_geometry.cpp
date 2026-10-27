#include "../../graphics/primitives/gouraud_array_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
#include <cmath>
namespace nfs::game::world {

void transform_start_line_point(const float *point, const float *rotation, const float *translation,
                                float *output) {
    float transformed[3];
    runtime::matrices::transform_positions(1, point, rotation, translation, transformed, 12);
    std::memcpy(output, transformed, 12);
}

float *sample_start_line_point(void *line, float *output, const float *local) {
    Dword contact[21]{};
    float position[3], surface[3];
    runtime::matrices::transform_positions(1, local, &at<float>(line, 24), &at<float>(line, 12),
                                           position, 12);
    std::memcpy(surface, position, 12);
    runtime::physics::surface_height(contact, surface);
    output[0] = surface[0];
    output[1] =
        float(kernel::numeric::Extended(surface[1], kernel::numeric::current_format()) + .04f);
    output[2] = -surface[2];
    return output;
}

void pending::build_start_line_geometry(Dword *line) {
    if (!graphics::device || line[1])
        return;
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float f) { return Extended(f, format); };
    Dword contact[21]{};
    if (!runtime::physics::surface_height(contact, &at<float>(line, 12)))
        return;
    float position[3];
    std::memcpy(position, bytes(line) + 12, 12);
    float step[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        step[axis] = float(E(at<float>(line, 24 + axis * 4)) * 1.5f);
    float positive = 0, negative = 0;
    for (int count = 0; count < 20; ++count) {
        if (!runtime::physics::surface_height(contact, position))
            break;
        positive = float(E(positive) + 1.5f);
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = float(E(position[axis]) + step[axis]);
    }
    std::memcpy(position, bytes(line) + 12, 12);
    for (int count = 0; count < 20; ++count) {
        if (!runtime::physics::surface_height(contact, position))
            break;
        negative = float(E(negative) - 1.5f);
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = float(E(position[axis]) - step[axis]);
    }
    auto span = E(positive) - negative;
    if (span < E(0))
        span = -span;
    if (!(E(4) < span) && !std::isnan(double(span)))
        return;
    negative = float(E(negative) + 2);
    const int cells = int(Dword(((span - 4) * 0.666666686534881591796875f).truncate_to_integer()));
    auto *memory =
        static_cast<Dword *>(lifecycle::allocate(2776, "StartLine", lifecycle::cars_pool | 1024));
    Dword *geometry = nullptr;
    if (memory) {
        try {
            geometry = graphics::gouraud::construct(memory, Dword(cells) * 12,
                                                    lifecycle::cars_pool | 1024, 4);
        } catch (...) {
            gui::application_release(memory);
            throw;
        }
    }
    line[1] = reinterpret_cast<Dword>(geometry);
    Dword state[17];
    lifecycle::construction::render_state(state);
    try {
        using namespace graphics::geoprim;
        primitive(state, 4);
        texture_enable(state, 0);
        shading(state, 1);
        alpha_test(state, 0);
        alpha_blend_mode(state, 1);
        transparency(state, 1);
        culling(state, 0);
        depth_method(state, 4);
        transparency(state, 1);
        graphics::gouraud::set_state(geometry, state);
        graphics::gouraud::set_matrix(geometry, runtime::skids::lifecycle::identity_matrix);
        float local[3] = {negative, 0, 0};
        const float cell_step[3] = {1.5f, 0, 0};
        float corners[8][3]{};
        Dword vertex = 0;
        for (int cell = 0; cell < cells; ++cell) {
            const float plus_x = float(E(local[0]) + cell_step[0]);
            const float plus_y = float(E(cell_step[1]) + local[1]);
            const float plus_z = float(E(cell_step[2]) + local[2]);
            float locations[7][3] = {
                {local[0], local[1], float(E(local[2]) + 1.5f)},
                {plus_x, plus_y, float(E(float(E(local[2]) + 1.5f)) + cell_step[2])},
                {plus_x, plus_y, plus_z},
                {local[0], local[1], local[2]},
                {local[0], local[1], float(E(local[2]) - 1.5f)},
                {plus_x, plus_y, float(E(float(E(local[2]) - 1.5f)) + cell_step[2])},
                {plus_x, plus_y, plus_z}};
            for (unsigned point = 0; point < 7; ++point) {
                Dword ground[21];
                runtime::physics::pending::construct_ground_contact(ground);
                kernel::memory_zero(ground, sizeof ground);
                float transformed[3], surface[3];
                if (point < 5)
                    runtime::matrices::transform_positions(1, locations[point],
                                                           &at<float>(line, 24),
                                                           &at<float>(line, 12), transformed, 12);
                else
                    transform_start_line_point(locations[point], &at<float>(line, 24),
                                               &at<float>(line, 12), transformed);
                std::memcpy(surface, transformed, 12);
                runtime::physics::surface_height(ground, surface);
                corners[point][0] = surface[0];
                corners[point][1] = float(E(surface[1]) + .04f);
                corners[point][2] = -surface[2];
            }
            sample_start_line_point(line, corners[7], local);
            const Dword *first = line + (cell & 1 ? 60 / 4 : 64 / 4);
            const Dword *second = line + (cell & 1 ? 64 / 4 : 60 / 4);
            const unsigned first_indices[6] = {0, 1, 2, 2, 3, 0};
            const unsigned second_indices[6] = {4, 5, 6, 6, 7, 4};
            for (unsigned index : first_indices)
                graphics::gouraud::set_vertex(geometry, vertex++, corners[index], first);
            for (unsigned index : second_indices)
                graphics::gouraud::set_vertex(geometry, vertex++, corners[index], second);
            runtime::physics::accumulate_triplet(local, cell_step);
        }
        graphics::gouraud::commit(geometry);
    } catch (...) {
        lifecycle::construction::destruction::render_state_member(state);
        throw;
    }
    lifecycle::construction::destruction::render_state_member(state);
}
} // namespace nfs::game::world
