#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::backend {
// Stages beyond tail cache one while passing the live input operation.
void apply_texture_delta(const TextureStageState *input) {
    if (bound_pixel_shader())
        return;
    static const Dword names[17] = {1, 26, 2, 3, 4, 27, 5, 6, 28, 7, 8, 9, 10, 11, 22, 23, 24};
    Dword stage = 0;
    do {
        const Dword requested_operation = input->values[0][stage];
        if (requested_operation != texture_stage_state.values[0][stage]) {
            texture_stage_state.values[0][stage] = requested_operation;
            // The cache may overlap the source; the native COM argument is a fresh read.
            com(backend_device, 252, stage, Dword(1), input->values[0][stage]);
            // The virtual call precedes a second input read in the function.
            const Dword operation = input->values[0][stage];
            if ((operation == 2 && input->values[2][stage] == 0) ||
                (operation == 3 && input->values[3][stage] == 0)) {
                unresolved::texture_values[stage] = 0;
                com(backend_device, 244, stage, Dword(0));
            }
        }
        for (int row = 1; row < 17; ++row) {
            const Dword value = input->values[row][stage];
            if (value != texture_stage_state.values[row][stage]) {
                texture_stage_state.values[row][stage] = value;
                com(backend_device, 252, stage, names[row], input->values[row][stage]);
            }
        }
        ++stage;
    } while (stage <= input->tail);
    for (; stage < 8; ++stage) {
        if (texture_stage_state.values[0][stage] != 1) {
            texture_stage_state.values[0][stage] = 1;
            com(backend_device, 252, stage, Dword(1), input->values[0][stage]);
        }
    }
}
} // namespace nfs::graphics::backend
