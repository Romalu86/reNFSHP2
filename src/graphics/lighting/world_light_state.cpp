#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::lifecycle::runtime {

void reset_world_lights(Dword *state) {
    state[3] = state[4] = state[5] = 1065353216;
    for (unsigned i = 6; i <= 14; ++i)
        state[i] = 0;
    state[15] = 1056964608;
    state[16] = 0;
    state[17] = 1176255488;
    state[18] = 0;
    Dword *light = state + 88 / 4;
    for (int i = 0; i < 3; ++i, light += 7) {
        light[0] = light[1] = light[2] = light[-3] = light[-2] = light[-1] = 0;
        light[3] = 1065353216;
    }
    state[76 / 4] = state[80 / 4] = state[84 / 4] = 0;
    state[100 / 4] = state[88 / 4] = 1065353216;
    state[92 / 4] = state[96 / 4] = 0;
}
} // namespace nfs::lifecycle::runtime
namespace nfs::lifecycle::table_entries {

void __fastcall initialize_world_light(System *state, void *) {
    runtime::reset_world_lights(reinterpret_cast<Dword *>(state));
}

void __fastcall enter_world_light_frontend(System *state, void *) {
    runtime::reset_world_lights(reinterpret_cast<Dword *>(state));
}

void __fastcall leave_world_light_frontend(System *state, void *) {
    runtime::reset_world_lights(reinterpret_cast<Dword *>(state));
}
} // namespace nfs::lifecycle::table_entries
