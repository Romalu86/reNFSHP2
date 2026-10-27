#include "model_render_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::graphics::model_render {
Dword *parameters{}, *parameter_start{}, *commands{};
Dword index_count{}, vertex_count{}, primitive_count{}, retained_buffers{}; // 4FC/508/504/50C.
Dword primitive_state[17]{};
Dword *technique_owner{}, *technique_state = primitive_state;
Dword dynamic_usage{}, index_offset{};
float bounds_matrix[16]{}, transposed_matrix[16]{};
Dword *active_target{};
Byte identity_transforms{};
namespace {
#include "../resources/eagl_primitive_counts.inc"
Dword inferred_count(Dword count) {
    const Dword *rule = primitive_counts[geoprim::current_primitive];
    return Dword(static_cast<int>(count - rule[1]) / static_cast<int>(rule[0]));
}
void draw_passes(bool indexed, bool inferred, bool nested, bool bind_before_pass) {
    Dword count;
    if (indexed && !nested)
        backend::index_base(index_offset);
    techniques::begin(technique_owner, &count, reinterpret_cast<Dword>(technique_state));
    Dword *saved = commands;
    if (nested)
        ++commands;
    for (int pass = 0; pass < static_cast<int>(count); ++pass) {
        if (nested) {
            Dword instruction = *commands;
            while (instruction && instruction >> 16 != 68) {
                command_table[instruction >> 16]();
                commands += instruction & 65535;
                instruction = *commands;
            }
        }
        if (indexed && nested && bind_before_pass)
            backend::index_base(index_offset);
        techniques::end(technique_owner, Dword(pass), reinterpret_cast<Dword>(technique_state));
        geoprim::apply(technique_state);
        if (indexed) {
            if (nested && !bind_before_pass)
                backend::index_base(index_offset);
            backend::bind_indices();
            backend::draw_indexed(0, index_count, 0, vertex_count,
                                  inferred ? inferred_count(index_count) : primitive_count);
        } else {

            Dword inferred_source = nested ? index_count : vertex_count;
            backend::draw_primitives(0, vertex_count,
                                     inferred ? inferred_count(inferred_source) : primitive_count);
        }
    }
    techniques::finish(technique_owner);
    if (nested)
        commands = saved;
}
void constant_words(bool pixel) {
    Dword count = Dword(static_cast<int>(commands[2]) >> 2);
    if (pixel)
        backend::pixel_constants(commands[1], parameters[1], count);
    else
        backend::vertex_constants(commands[1], parameters[1], count);
}
void matrix_constants(bool pixel) {
    pending::d3dx_transpose(transposed_matrix, pointer<float>(parameters[1]));
    if (pixel)
        backend::pixel_constants(commands[1], reinterpret_cast<Dword>(transposed_matrix), 4);
    else
        backend::vertex_constants(commands[1], reinterpret_cast<Dword>(transposed_matrix), 4);
}
void dynamic_stream(bool indirect) {
    Dword *instruction = commands;
    const Dword stage = instruction[1];
    Dword handle = instruction[4];
    const Dword capacity = instruction[5], layout = instruction[3], stride = instruction[2];
    const Dword count = indirect ? parameter_start[instruction[10] * 2] : parameters[0];
    const Dword bytes = count * stride;
    if (static_cast<int>(capacity) < static_cast<int>(count)) {
        if (instruction[4] != ~0u)
            resources::release_vertex(&handle, int(stage));
        handle = reinterpret_cast<Dword>(
            buffers::create_vertex(bytes, stride, 520, layout, dynamic_usage, bytes * 4));
        commands[4] = handle;
        instruction = commands;
        instruction[5] =
            (indirect ? parameter_start[instruction[10] * 2] : parameters[0]) << 2;
    }
    if (!retained_buffers) {
        const Dword data = indirect ? parameter_start[commands[10] * 2 + 1] : parameters[1];
        buffers::replace_data(pointer<Dword>(handle), data, bytes);
    }
    backend::bind_stream(stage, pointer<Dword>(handle));
}
void set_transform(Dword kind, bool reset_identity) {
    backend::com(backend_device, 148, kind, parameter_start[commands[1] * 2 + 1]);
    if (reset_identity)
        identity_transforms = 0;
}
} // namespace

Dword *selected_target() {
    return active_target;
}
 
void unused_command() {
}
 
void command_marker() {
}

void command_vertex_constants() {
    constant_words(false);
}

void command_vertex_constants_variant() {
    constant_words(false);
}
 
void command_stream_marker() {
}

void command_static_stream() {
    backend::bind_stream(
        commands[1], pointer<Dword>(pointer<Dword>(commands[4])[buffers::selection_index * 2 + 1]));
}

void command_dynamic_stream() {
    dynamic_stream(false);
}

void command_dynamic_indices() {
    const Dword element_size = commands[1], data = parameters[1];
    const Dword bytes = element_size * parameters[0];
    Dword handle = commands[2];
    if (handle == ~0u || static_cast<int>(bytes) > static_cast<int>(pointer<Dword>(handle)[2])) {
        if (handle != ~0u)
            resources::release_index(&handle);
        handle = reinterpret_cast<Dword>(buffers::create_index(bytes * 4, dynamic_usage, 520));
        commands[2] = handle;
    }
    if (!retained_buffers) {
        Dword mapped;
        resources::index_lock(pointer<Dword>(handle), 0, bytes, &mapped, 8192);
        kernel::memory_copy(pointer<void>(mapped), pointer<void>(data), bytes);
        resources::index_unlock(pointer<Dword>(handle));
    }
    backend::index_buffer(pointer<Dword>(handle));
}

void command_static_indices() {
    backend::index_buffer(pointer<Dword>(commands[3]));
}

void command_texture() {
    backend::unresolved::texture_cache[commands[1]] =
        commands[2] * buffers::selection_index + parameters[1];
}

void command_geoprim() {
    technique_state = pointer<Dword>(parameters[1]);
}

void command_vertex_matrix() {
    matrix_constants(false);
}
 
void command_vertex_program_marker() {
}
 
void command_pixel_program_marker() {
}

void command_draw() {
    draw_passes(false, false, false, false);
}

void command_draw_indexed() {
    draw_passes(true, false, false, false);
}

void command_draw_indexed_variant() {
    draw_passes(true, false, false, false);
}

void command_constant_array() {
    constant_words(false);
}

void command_constant_array_variant() {
    constant_words(false);
}

void command_texture_variant() {
    command_texture();
}

void command_local_geoprim() {
    geoprim::assign(primitive_state, pointer<Dword>(parameters[1]));
    technique_state = primitive_state;
}

void command_vertex_matrix_variant() {
    matrix_constants(false);
}

void command_draw_inferred() {
    draw_passes(false, true, false, false);
}

void command_draw_indexed_inferred() {
    draw_passes(true, true, false, false);
}

void command_draw_indexed_inferred_variant() {
    draw_passes(true, true, false, false);
}

void command_pixel_matrix() {
    matrix_constants(true);
}

void command_pixel_matrix_variant() {
    matrix_constants(true);
}

void command_pixel_constants() {
    constant_words(true);
}

void command_pixel_constants_variant() {
    constant_words(true);
}

void command_world_transform() {
    set_transform(256, true);
}

void command_view_transform() {
    set_transform(2, true);
}

void command_texture_transform0() {
    set_transform(16, false);
}

void command_texture_transform1() {
    set_transform(17, false);
}

void command_texture_transform2() {
    set_transform(18, false);
}

void command_texture_transform3() {
    set_transform(19, false);
}

void command_texture_transform4() {
    set_transform(20, false);
}

void command_texture_transform5() {
    set_transform(21, false);
}

void command_texture_transform6() {
    set_transform(22, false);
}

void command_texture_transform7() {
    set_transform(23, false);
}

void command_projection_transform() {
    set_transform(3, true);
}

void command_combined_transform() {
    set_transform(256, false);
    if (!identity_transforms) {
        backend::com(backend_device, 148, Dword(3), backend::identity);
        backend::com(backend_device, 148, Dword(2), backend::identity);
    }
    identity_transforms = 1;
}

void command_nested_draw() {
    draw_passes(false, false, true, false);
}

void command_nested_indexed() {
    draw_passes(true, false, true, true);
}

void command_nested_draw_inferred() {
    draw_passes(false, true, true, false);
}

void command_nested_indexed_inferred() {
    draw_passes(true, true, true, false);
}
 
void command_skin_marker() {
}

void command_indirect_stream() {
    dynamic_stream(true);
}

void set_command_texture_factor() {
    auto *colour = pointer<float>(parameter_start[commands[1] * 2 + 1]);
    auto convert = [&](unsigned component) {

        return Dword(static_cast<long long>(static_cast<long double>(colour[component]) * 255.0f));
    };
    Dword packed = convert(3);
    packed = (packed << 8) | convert(0);
    packed = (packed << 8) | convert(1);
    packed = (packed << 8) | convert(2);
    backend::texture_factor(packed);
}

const Command command_table[78] = {unused_command,
                                   command_marker,
                                   command_vertex_constants,
                                   command_stream_marker,
                                   command_static_stream,
                                   command_dynamic_stream,
                                   unused_command,
                                   command_static_indices,
                                   command_dynamic_indices,
                                   command_texture,
                                   command_geoprim,
                                   unused_command,
                                   command_vertex_matrix,
                                   unused_command,
                                   command_vertex_program_marker,
                                   command_pixel_program_marker,
                                   command_draw,
                                   command_draw_indexed,
                                   command_draw_indexed_variant,
                                   upload_skin_constants,
                                   upload_skin_constants_sse,
                                   upload_skin_constants_packed,
                                   transpose_skin_constants,
                                   transpose_skin_constants_sse,
                                   transpose_skin_constants_packed,
                                   unused_command,
                                   unused_command,
                                   unused_command,
                                   set_skin_palette,
                                   unused_command,
                                   command_vertex_constants_variant,
                                   command_constant_array,
                                   command_texture_variant,
                                   command_local_geoprim,
                                   unused_command,
                                   command_vertex_matrix_variant,
                                   unused_command,
                                   upload_skin_constants_variant,
                                   upload_skin_constants_sse_variant,
                                   upload_skin_constants_packed_variant,
                                   transpose_skin_constants_variant,
                                   transpose_skin_constants_sse_variant,
                                   transpose_skin_constants_packed_variant,
                                   unused_command,
                                   unused_command,
                                   unused_command,
                                   command_constant_array_variant,
                                   command_draw_inferred,
                                   command_draw_indexed_inferred,
                                   command_draw_indexed_inferred_variant,
                                   pending::configure_lights,
                                   command_world_transform,
                                   command_view_transform,
                                   command_texture_transform0,
                                   command_texture_transform1,
                                   command_texture_transform2,
                                   command_texture_transform3,
                                   command_texture_transform4,
                                   command_texture_transform5,
                                   command_texture_transform6,
                                   command_texture_transform7,
                                   command_projection_transform,
                                   command_combined_transform,
                                   unused_command,
                                   command_nested_draw,
                                   command_nested_indexed,
                                   command_nested_draw_inferred,
                                   command_nested_indexed_inferred,
                                   unused_command,
                                   pending::configure_draw_pipeline,
                                   command_pixel_matrix,
                                   command_pixel_matrix_variant,
                                   command_pixel_constants,
                                   command_pixel_constants_variant,
                                   command_skin_marker,
                                   pending::skin_vertex_stream,
                                   command_indirect_stream,
                                   set_command_texture_factor};

void draw_method(void *method, Dword *record) {
    backend::selected_device();
    backend::selected_device();
    Dword *record_fields = record + 1;
    for (int i = 7; i >= 0; --i)
        backend::unresolved::texture_cache[i] = 0;
    parameters = record_fields;
    parameter_start = record_fields;
    int selection = field<int>(method, 40);
    int technique = selection > -1 ? *pointer<short>(record_fields[selection * 2 + 1]) : 0;
    field<int>(pointer<void>(field<Dword>(method, 12)), 32) = technique;
    technique_owner = pointer<Dword>(field<Dword>(method, 12));
    vertex_count = pointer<Dword>(record_fields[1])[0];
    index_count = pointer<Dword>(record_fields[1])[1];
    primitive_count = pointer<Dword>(record_fields[1])[2];
    retained_buffers = pointer<Dword>(record_fields[1])[3];
    commands = pointer<Dword>(pointer<Dword>(field<Dword>(method, 4))[technique]);
    Dword instruction = *commands;
    command_table[instruction >> 16]();
    commands += instruction & 65535;
    while ((instruction = *commands) != 0) {
        command_table[instruction >> 16]();
        commands += instruction & 65535;
        parameters += 2;
    }
}
} // namespace nfs::graphics::model_render
