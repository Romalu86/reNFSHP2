#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "../../graphics/actors/model_render_runtime.hpp"
namespace nfs::graphics::immediate {

void submit_dynamic_model(Dword *model) {
    for (int index = 0; index < int(model[72 / 4]); ++index) {
        auto *builders = reinterpret_cast<Dword **>(model[80 / 4]);
        Dword *builder = builders[index];
        if (builder) {
            model_builder_matrix(builder, model);
            builder = reinterpret_cast<Dword **>(model[80 / 4])[index];
            render_model_builder(builder, builder[56 / 4]);
        } else {
            auto *record = reinterpret_cast<Dword **>(model[76 / 4])[index];
            model_render::draw_method(reinterpret_cast<void *>(record[0]), record);
        }
    }
}

void draw_dynamic_model(Dword *model) {
    for (unsigned index = 0; index < 16; ++index)
        backend::model_matrix[index] = model[index];
    float combined[16];
    math::copy(combined, reinterpret_cast<float *>(backend::view_matrix));
    model_render::prepend_matrix(combined, reinterpret_cast<float *>(backend::model_matrix));
    kernel::memory_copy(backend::model_view_matrix, combined, 64);
    math::copy(combined, reinterpret_cast<float *>(backend::view_projection_matrix));
    model_render::prepend_matrix(combined, reinterpret_cast<float *>(backend::model_matrix));
    buffers::selection_index = 0;
    kernel::memory_copy(backend::model_view_projection_matrix, combined, 64);
    submit_dynamic_model(model);
}

std::uint8_t render_model_builder(Dword *builder, Dword value) {
    if (reinterpret_cast<std::uint8_t *>(builder)[24])
        return 0;
    builder[28 / 4] = value;
    model_builder_bindings(builder, 0);
    draw_dynamic_model(reinterpret_cast<Dword *>(builder[1]));
    reinterpret_cast<std::uint8_t *>(builder)[52] = 0;
    return 1;
}
} // namespace nfs::graphics::immediate
