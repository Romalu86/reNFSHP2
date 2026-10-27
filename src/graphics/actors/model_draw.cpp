#include "model_render_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::graphics::model_render {
// The argument is the left operand; this is the right operand.
void prepend_matrix(float *matrix, const float *left) {
    float combined[16];
    math::multiply(left, matrix, combined);
    kernel::memory_copy(matrix, combined, sizeof combined);
}

void patch_values(const Dword *patches) {
    if (patches && static_cast<int>(*patches) > 0) {
        Dword count = *patches++;
        do {
            Dword *destination = pointer<Dword>(patches[0]);
            Dword value = patches[1];
            patches += 2;
            *destination = value;
        } while (--count);
    }
}

void set_bounds(const float *lower, const float *upper) {
    for (unsigned i = 0; i < 12; ++i)
        bounds_matrix[i] = i % 5 == 0 ? lower[i / 5] : 0.0f;
    for (unsigned i = 0; i < 3; ++i)
        bounds_matrix[12 + i] = upper[i] - lower[i];
    bounds_matrix[15] = 1.0f;
}
// The caller consumes AL only.
Byte sphere_visible(Dword *viewport, const float *centre, float radius) {
    float matrix[16], position[3];
    math::copy(matrix, reinterpret_cast<float *>(backend::view_matrix));
    math::transform_position(centre, matrix, position);
    auto X = [](float value) { return static_cast<long double>(value); };
    if (X(radius) < X(position[2]) + field<float>(viewport, 276))
        return 0;
    if (X(radius) < -(X(position[2]) + field<float>(viewport, 280)))
        return 0;
    if (!field<Dword>(viewport, 44)) {
        auto horizontal = X(position[2]) * field<float>(viewport, 316) + position[0];
        if (X(0) < horizontal) {
            horizontal = horizontal * field<float>(viewport, 320);
            if (X(radius) < horizontal)
                return 0;
        } else {
            const auto left = X(position[2]) * field<float>(viewport, 308) + position[0];

            if (left < X(0) && X(radius) < -(left * field<float>(viewport, 312)))
                return 0;
        }
        auto vertical = X(position[2]) * field<float>(viewport, 324) + position[1];
        if (X(0) < vertical) {
            vertical = vertical * field<float>(viewport, 328);
            if (X(radius) < vertical)
                return 0;
        } else {
            const auto bottom = X(position[2]) * field<float>(viewport, 332) + position[1];
            // The bottom-plane path likewise has no intervening binary32 store.
            if (bottom < X(0) && X(radius) < -(bottom * field<float>(viewport, 336)))
                return 0;
        }
    }
    return 1;
}
} // namespace nfs::graphics::model_render
namespace nfs::game::world::vehicle_render::pending {

void draw_model(void *model, const Dword *transform) {
    using namespace graphics;
    if (!at<Dword>(model, 200))
        return;
    auto *selected = backend::selected_device();

    Dword *target = model_render::selected_target();
    Dword *viewport =
        target ? viewports::target_viewport(target)
               : viewports::current(backend::selected_context(backend::selected_device()));
    (void)selected;
    if (viewports::data(viewport)) {
        auto X = [](float value) { return static_cast<long double>(value); };
        const float *matrix = reinterpret_cast<const float *>(transform);
        float centre[3];
        for (unsigned axis = 0; axis != 3; ++axis)
            centre[axis] = float(X(at<float>(model, 148)) * matrix[8 + axis] +
                                 X(at<float>(model, 144)) * matrix[4 + axis] +
                                 X(at<float>(model, 140)) * matrix[axis] + matrix[12 + axis]);
        if (!model_render::sphere_visible(viewport, centre, at<float>(model, 152)))
            return;
    }
    for (unsigned i = 0; i != 16; ++i)
        backend::model_matrix[i] = transform[i];
    float combined[16];
    math::copy(combined, reinterpret_cast<float *>(backend::view_matrix));
    model_render::prepend_matrix(combined, reinterpret_cast<float *>(backend::model_matrix));
    kernel::memory_copy(backend::model_view_matrix, combined, 64);
    math::copy(combined, reinterpret_cast<float *>(backend::view_projection_matrix));
    model_render::prepend_matrix(combined, reinterpret_cast<float *>(backend::model_matrix));
    kernel::memory_copy(backend::model_view_projection_matrix, combined, 64);
    if (at<Dword *>(model, 208))
        model_render::patch_values(at<Dword *>(model, 208));
    buffers::selection_index = at<Dword>(model, 192);
    model_render::set_bounds(reinterpret_cast<float *>(offset(model, 76)),
                             reinterpret_cast<float *>(offset(model, 92)));
    if (Dword *directory = at<Dword *>(model, 204)) {
        Dword threshold = *directory++ & 65535, kind = threshold, layer = 0;
        do {
            if (kind == 65535) {
                Dword *record = reinterpret_cast<Dword *>(*directory++);
                if (record)
                    model_render::draw_method(reinterpret_cast<void *>(*record), record);
            } else {
                if (!at<short>(at<void *>(model, 196), layer + 2))
                    directory += directory[1] + 2;
                else
                    directory += 2;
                layer += 4;
            }
            kind = *directory++ & 65535;
        } while (threshold < kind);
    }
    void *inherited_child = at<void *>(model, 164);
    void *composed_child = at<void *>(model, 172);
    void *local_child = at<void *>(model, 168);

    if (!inherited_child && !composed_child && !local_child)
        return;
    for (void *child = inherited_child; child; child = at<void *>(child, 176))
        draw_model(child, transform);
    for (void *child = at<void *>(model, 168); child; child = at<void *>(child, 176))
        draw_model(child, reinterpret_cast<Dword *>(offset(child, 12)));
    for (void *child = at<void *>(model, 172); child; child = at<void *>(child, 176)) {
        kernel::memory_copy(combined, offset(child, 12), 64);
        model_render::prepend_matrix(combined, reinterpret_cast<const float *>(transform));
        draw_model(child, reinterpret_cast<Dword *>(combined));
    }
}
} // namespace nfs::game::world::vehicle_render::pending
