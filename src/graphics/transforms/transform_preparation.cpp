#include "nfs/graphics/transforms/transform_handlers.hpp"
#include <cstring>
namespace nfs::game::runtime::matrices::handlers {
// Keep the packed-state instruction after all cache writes. A separate C++
// contribution prevents the compiler from scheduling it before those writes.
void finish_packed_state();
alignas(16) Dword prepared_matrix[12]{}, prepared_vectors[12]{};
namespace {
Dword representation(const float *value) {
    Dword bits;
    std::memcpy(&bits, value, 4);
    return bits;
}
} // namespace
// The provider consumes these persistent cache fields.
// The native register-only boundary remains a separate ABI obligation.
void prepare_packed() {
    if (matrix_changed) {
        const float *selected_matrix = matrix;
        matrix_changed = 0;
        for (unsigned axis = 0; axis < 3; ++axis)
            for (unsigned column = 0; column < 3; ++column)
                static_cast<volatile Dword *>(prepared_matrix)[axis * 4 + column] =
                    representation(selected_matrix + axis + column * 3);
    }
    if (translation_changed) {
        const float *selected_translation = translation;
        translation_changed = 0;
        auto *output = static_cast<volatile Dword *>(prepared_vectors);
        output[0] = representation(selected_translation);
        output[1] = representation(selected_translation + 1);
        Dword z = representation(selected_translation + 2);
        output[2] = z;
        output[3] = z;
    }
    if (projection_changed) {
        projection_changed = 0;
        auto *output = static_cast<volatile Dword *>(prepared_vectors);
        output[4] = representation(&screen_x);
        output[5] = representation(&screen_y) ^ 2147483648u;
        output[8] = representation(&offset_x);
        output[9] = representation(&offset_y);
    }
    finish_packed_state();
}
// SSE register outputs are represented by the explicit cache
// consumed at the pending numeric-provider boundary; raw register ABI is open.
void prepare_streaming() {
    if (matrix_changed) {
        const float *selected_matrix = matrix;
        matrix_changed = 0;
        for (unsigned axis = 0; axis < 3; ++axis)
            for (unsigned column = 0; column < 3; ++column)
                static_cast<volatile Dword *>(prepared_matrix)[axis * 4 + 2 - column] =
                    representation(selected_matrix + axis * 3 + column);
    }
    if (translation_changed) {
        const float *selected_translation = translation;
        translation_changed = 0;
        for (unsigned axis = 0; axis < 3; ++axis)
            static_cast<volatile Dword *>(prepared_vectors)[2 - axis] =
                representation(selected_translation + axis);
    }
    if (projection_changed) {
        projection_changed = 0;
        auto *output = static_cast<volatile Dword *>(prepared_vectors);
        output[6] = representation(&screen_x);
        output[5] = representation(&screen_y) ^ 2147483648u;
        output[10] = representation(&offset_x);
        output[9] = representation(&offset_y);
    }
}
} // namespace nfs::game::runtime::matrices::handlers
