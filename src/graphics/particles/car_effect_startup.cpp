#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include <cstdlib>

namespace nfs::game::runtime::particles::lifecycle {

void destroy_car_alpha_particles() {
    nfs::lifecycle::construction::destruction::render_state_member(&state<Dword>(7216576));
}

 
void initialize_car_alpha_particles() {
    nfs::lifecycle::construction::render_state(&state<Dword>(7216576));
    std::atexit(destroy_car_alpha_particles);
}


void destroy_car_additive_particles() {
    nfs::lifecycle::construction::destruction::render_state_member(&state<Dword>(7216504));
}

 
void initialize_car_additive_particles() {
    nfs::lifecycle::construction::render_state(&state<Dword>(7216504));
    std::atexit(destroy_car_additive_particles);
}


void destroy_car_alpha_splatters() {
    nfs::lifecycle::construction::destruction::render_state_member(&state<Dword>(7216432));
}

 
void initialize_car_alpha_splatters() {
    nfs::lifecycle::construction::render_state(&state<Dword>(7216432));
    std::atexit(destroy_car_alpha_splatters);
}


void destroy_car_additive_splatters() {
    nfs::lifecycle::construction::destruction::render_state_member(&state<Dword>(7216360));
}

 
void initialize_car_additive_splatters() {
    nfs::lifecycle::construction::render_state(&state<Dword>(7216360));
    std::atexit(destroy_car_additive_splatters);
}
} // namespace nfs::game::runtime::particles::lifecycle

NFS_KERNEL_INITIALIZER(startup_initialize_car_alpha_particles, ".CRT$XCU_NFS_00690010",
                       nfs::game::runtime::particles::lifecycle::initialize_car_alpha_particles);
NFS_KERNEL_INITIALIZER(startup_initialize_car_additive_particles, ".CRT$XCU_NFS_00690014",
                       nfs::game::runtime::particles::lifecycle::initialize_car_additive_particles);
NFS_KERNEL_INITIALIZER(startup_initialize_car_alpha_splatters, ".CRT$XCU_NFS_00690018",
                       nfs::game::runtime::particles::lifecycle::initialize_car_alpha_splatters);
NFS_KERNEL_INITIALIZER(startup_initialize_car_additive_splatters, ".CRT$XCU_NFS_0069001C",
                       nfs::game::runtime::particles::lifecycle::initialize_car_additive_splatters);
