#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::graphics {

Dword positional_lights[28]{};

// Keep this storage separate from positional_lights.
static Dword startup_matrix_state[28]{};
 
void initialize_positional_light_sources() {
    for (Dword &value : positional_lights)
        value = 0;
    positional_lights[0] = 1065353216;
    positional_lights[4] = 0;
    positional_lights[8] = 0;
    positional_lights[1] = 0;
    positional_lights[5] = 1065353216;
    positional_lights[9] = 0;
    positional_lights[2] = 0;
    positional_lights[6] = 0;
    positional_lights[10] = 1065353216;
}
 
void initialize_matrix_state() {
    for (Dword &value : startup_matrix_state)
        value = 0;
    startup_matrix_state[0] = 1065353216;
    startup_matrix_state[4] = 0;
    startup_matrix_state[8] = 0;
    startup_matrix_state[1] = 0;
    startup_matrix_state[5] = 1065353216;
    startup_matrix_state[9] = 0;
    startup_matrix_state[2] = 0;
    startup_matrix_state[6] = 0;
    startup_matrix_state[10] = 1065353216;
}
} // namespace nfs::graphics
