#include "model_render_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::graphics::model_render {
namespace {
Dword ambient_colour{};
Dword lights[3][26]{};
float cached_frustum[6]{};
float fog_start = -1, fog_end = -1;
float as_float(Dword bits) {
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}
Dword as_word(float value) {
    Dword bits;
    std::memcpy(&bits, &value, 4);
    return bits;
}
bool changed_value(float a, float b) {
    return a != b;
}
void render_state(Dword kind, Dword value) {
    backend::com(backend_device, 200, kind, value);
}
Dword rounded_colour(float value) {
    const auto format = kernel::numeric::current_format();
    float scaled = float(kernel::numeric::Extended(value, format) * 255.0f);
    auto integer = kernel::numeric::Extended(scaled, format).round_to_integer();
    return integer < -2147483648LL || integer > 2147483647LL ? 2147483648u : Dword(integer);
}
void update_fog_start(float value) {
    if (changed_value(fog_start, value)) {
        fog_start = value;
        render_state(36, as_word(value));
    }
}
void update_fog_end(float value) {
    if (changed_value(fog_end, value)) {
        fog_end = value;
        render_state(37, as_word(value));
    }
}
bool changed_projected_depth(float cached, long double depth) {
    return static_cast<long double>(cached) != depth;
}
void update_projected_fog_start(long double depth) {

    if (changed_projected_depth(fog_start, depth)) {
        fog_start = float(depth);
        render_state(36, as_word(fog_start));
    }
}
void update_projected_fog_end(long double depth) {

    if (changed_projected_depth(fog_end, depth)) {
        fog_end = float(depth);
        render_state(37, as_word(fog_end));
    }
}
} // namespace

void viewport_near(Dword *viewport, Dword *output) {
    *output = pointer<Dword>(*viewport)[276 / 4];
}

void viewport_far(Dword *viewport, Dword *output) {
    *output = pointer<Dword>(*viewport)[280 / 4];
}

void viewport_bottom(Dword *viewport, Dword *output) {
    *output = pointer<Dword>(*viewport)[268 / 4];
}

void viewport_top(Dword *viewport, Dword *output) {
    *output = pointer<Dword>(*viewport)[272 / 4];
}

void transform_rows(const float *matrix, Dword count, const float *input, float *output) {
    if (count)
        do {
            math::transform_row(input, matrix, output);
            input += 4;
            output += 4;
        } while (--count);
}

void set_active_target(Dword *target) {
    active_target = target;
}

void pending::configure_lights() {
    using namespace backend::unresolved;
    Dword enabled = commands[2];
    int count = int(backend::adapter_caps()[708 / 4]);
    Byte lighting = enabled != 0;
    if (lighting != lighting_enabled) {
        lighting_enabled = lighting;
        render_state(137, lighting);
    }
    if (!enabled)
        return;
    void *parameters = pointer<void>(parameter_start[commands[1] * 2 + 1]);
    if (count < 1) {
        Dword colour = rounded_colour(field<float>(parameters, 108));
        colour = (colour << 8) | rounded_colour(field<float>(parameters, 96));
        colour = (colour << 8) | rounded_colour(field<float>(parameters, 100));
        colour = (colour << 8) | rounded_colour(field<float>(parameters, 104));
        if (ambient_colour != colour) {
            ambient_colour = colour;
            render_state(139, colour);
        }
        return;
    }
    if (count > 3)
        count = 3;
    for (int light = 0; light < count; ++light) {
        lights[light][0] = 3;
        std::memcpy(lights[light] + 1, static_cast<Byte *>(parameters) + 48 + light * 16, 16);
        std::memcpy(lights[light] + 9, static_cast<Byte *>(parameters) + 96, 16);
        lights[light][16] = as_word(-field<float>(parameters, Dword(light) * 4));
        lights[light][17] = as_word(-field<float>(parameters, 16 + Dword(light) * 4));
        lights[light][18] = as_word(-field<float>(parameters, 32 + Dword(light) * 4));
        backend::com(backend_device, 176, Dword(light), lights[light]);
    }
}

void pending::configure_draw_pipeline() {
    using namespace backend::unresolved;
    if (lighting_enabled) {
        lighting_enabled = 0;
        render_state(137, 0);
    }
    dynamic_usage = commands[1];
    bool software = true;
    if (dynamic_usage) {
        if (state::supports_hardware_fixed_function_processing())
            software = false;
        else if (!dynamic_usage && state::supports_hardware_vertex_programs())
            software = false;
    } else if (state::supports_hardware_vertex_programs())
        software = false;
    backend::set_software_processing(software);
    Dword *context = backend::selected_context(backend::selected_device());
    Byte fog;
    state::read_context_fog_enabled(context, &fog);
    if (fog != fog_enabled) {
        fog_enabled = fog;
        render_state(28, fog);
    }
    if (!fog) {
        index_offset = 0;
        return;
    }
    auto X = [](float value) { return static_cast<long double>(value); };
    Dword start_bits, end_bits, mode, density_bits;
    if (!dynamic_usage) {
        state::read_fog_start(context, &start_bits);
        state::read_fog_end(context, &end_bits);
        state::read_fog_density(context, &density_bits);
        float start = as_float(start_bits), end = as_float(end_bits);
        auto *parameters = reinterpret_cast<float *>(backend::fog_parameters);
        const auto inverse_range = X(1) / (X(end) - start);

        parameters[0] = float(X(-1) * inverse_range);
        parameters[1] = float(inverse_range * end);
        Dword *fog_context = backend::selected_context(backend::selected_device());
        state::read_context_vertex_fog_mode(fog_context, &mode);
        if (mode == 1 || mode == 2)
            parameters[3] =
                float(X(as_float(mode == 1 ? 1069066811 : 1067040353)) * as_float(density_bits));
        if (vertex_fog) {
            vertex_fog = 0;
            render_state(140, 0);
        }
        if (table_fog) {
            table_fog = 0;
            render_state(35, 0);
        }
        index_offset = 0;
        return;
    }
    Dword *caps = backend::adapter_caps();
    Dword vertex_mode, table_mode;
    state::read_context_vertex_fog_mode(context, &vertex_mode);
    state::read_context_table_fog_mode(context, &table_mode);
    if (vertex_mode != vertex_fog) {
        vertex_fog = vertex_mode;
        render_state(140, vertex_mode);
    }
    if (table_mode != table_fog) {
        table_fog = table_mode;
        render_state(35, table_mode);
    }
    if (!software && (caps[584 / 4] & 1048576) == 1048576) {
        state::read_fog_start(context, &start_bits);
        state::read_fog_end(context, &end_bits);
        update_fog_start(-as_float(start_bits));
        update_fog_end(-as_float(end_bits));
    } else {
        float frustum[4];
        viewport_near(viewports::current(context), reinterpret_cast<Dword *>(frustum));
        viewport_far(viewports::current(context), reinterpret_cast<Dword *>(frustum + 1));
        viewport_top(viewports::current(context), reinterpret_cast<Dword *>(frustum + 2));
        viewport_bottom(viewports::current(context), reinterpret_cast<Dword *>(frustum + 3));
        float positions[8]{};
        state::read_fog_start(context, reinterpret_cast<Dword *>(cached_frustum + 4));
        state::read_fog_end(context, reinterpret_cast<Dword *>(cached_frustum + 5));
        bool changed = false;
        for (unsigned i = 0; i < 4; ++i)
            changed = changed || changed_value(frustum[i], cached_frustum[i]);
        changed = changed || changed_value(positions[2], cached_frustum[4]) ||
                  changed_value(positions[6], cached_frustum[5]);
        if (changed) {
            for (unsigned i = 0; i < 4; ++i)
                cached_frustum[i] = frustum[i];
            cached_frustum[4] = positions[2];
            cached_frustum[5] = positions[6];
            positions[3] = positions[7] = 1;
            positions[2] = float(X(positions[2]) - frustum[0]);
            positions[6] = float(X(positions[6]) - frustum[0]);
            transform_rows(reinterpret_cast<float *>(backend::projection_matrix), 2, positions,
                           positions);
            update_projected_fog_start(X(positions[2]) / X(positions[3]));
            update_projected_fog_end(X(positions[6]) / X(positions[7]));
        }
    }
    index_offset = 0;
}
} // namespace nfs::graphics::model_render
