#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "ai_population_runtime.hpp"
#include "nfs/core/errors/assertions.hpp"
#include <cstdlib>
#include "ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword location_reuse_ticks = 300;
Dword traffic_reservation_queries = Dword(-1);
float chase_transition_seconds = 0.2f;
extern Dword threat_memory_ticks;

void initialize_chase_warmup() {
    chase_warmup_ticks = 180;
}

void initialize_helicopter_activation_delay() {
    helicopter_activation_ticks = 450;
}

void initialize_threat_memory() {
    threat_memory_ticks = 120;
}

void initialize_bust_contact_grace() {
    state<Dword>(7352416) = 15;
}

void initialize_cop_maximum_stun() {
    state<Dword>(7352432) = 300;
}

void initialize_cop_recovery_delay() {
    state<Dword>(7352428) = 120;
}

void initialize_cop_chase_delay() {
    state<Dword>(7352424) = 180;
}

void initialize_cop_boredom_delay() {
    state<Dword>(7352420) = 360;
}

void initialize_path_controller_pointer() {
    state<Dword *>(7352448) = ai::pending::path_controller;
}

const char *__fastcall traffic_location_type(void *, void *) {
    return "copLocation";
}

const char *__fastcall roadblock_location_type(void *, void *) {
    return "copRoadblockLocationManager";
}

const char *__fastcall traffic_manager_type(void *, void *) {
    return "copSpawnLocationManager";
}

const Dword location_manager_base_methods[] = {
    reinterpret_cast<Dword>(assertions::pure_virtual_call)};
const Dword traffic_location_base_methods[] = {
    reinterpret_cast<Dword>(traffic_location_type),
    reinterpret_cast<Dword>(assertions::pure_virtual_call),
    reinterpret_cast<Dword>(assertions::pure_virtual_call),
    reinterpret_cast<Dword>(assertions::pure_virtual_call)};
const Dword roadblock_location_base_methods[] = {
    reinterpret_cast<Dword>(assertions::pure_virtual_call)};
const Dword roadblock_location_methods[] = {reinterpret_cast<Dword>(roadblock_location_type),
                                           reinterpret_cast<Dword>(reserve_roadblock_location)};
const Dword traffic_location_methods[] = {reinterpret_cast<Dword>(traffic_manager_type),
                                         reinterpret_cast<Dword>(initialize_traffic_locations),
                                         reinterpret_cast<Dword>(reserve_traffic_location),
                                         reinterpret_cast<Dword>(reserve_nearest_traffic_location)};

void *construct_location_manager(void *owner, Dword type) {
    at<Dword>(owner, 4) = type;
    at<Dword>(owner, 8) = at<Dword>(owner, 12) = 0;
    at<const Dword *>(owner, 0) = location_manager_base_methods;
    at<Dword>(owner, 16) = at<Dword>(owner, 20) = at<Dword>(owner, 24) = 0;
    at<const Dword *>(owner, 0) = traffic_location_base_methods;
    return owner;
}

void *construct_traffic_locations(void *owner) {
    construct_location_manager(owner, 1);
    at<const Dword *>(owner, 0) = traffic_location_methods;
    return owner;
}

void destroy_location_manager(void *owner) {
    at<const Dword *>(owner, 0) = traffic_location_base_methods;
    at<const Dword *>(owner, 0) = location_manager_base_methods;
    if (void *records = at<void *>(owner, 12))
        kernel::memory_free(records);
}

void destroy_traffic_locations(void *owner) {
    at<const Dword *>(owner, 0) = traffic_location_methods;
    destroy_location_manager(owner);
}

void *construct_roadblock_locations(void *owner) {
    at<Dword>(owner, 4) = 3;
    at<Dword>(owner, 8) = at<Dword>(owner, 12) = 0;
    at<const Dword *>(owner, 0) = roadblock_location_base_methods;
    at<const Dword *>(owner, 0) = roadblock_location_methods;
    return owner;
}

void destroy_roadblock_locations(void *owner) {
    at<const Dword *>(owner, 0) = roadblock_location_methods;
    at<const Dword *>(owner, 0) = roadblock_location_base_methods;
    if (void *records = at<void *>(owner, 12))
        kernel::memory_free(records);
}

Dword *construct_spike_slots(Dword *slots) {
    slots[0] = slots[1] = 0;
    return slots;
}

void destroy_spike_slots(Dword *slots) {
    for (int i = 0; i < 2; ++i)
        if (slots[i])
            invoke(reinterpret_cast<void *>(slots[i]), 5, Dword{1});
}

Dword *construct_path_controller(Dword *controller) {
    for (unsigned offset : {80u, 82u, 84u, 86u, 88u})
        at<std::uint16_t>(controller, offset) = 0;
    construct_traffic_locations(bytes(controller) + 92);
    try {
        construct_roadblock_locations(bytes(controller) + 120);
        try {
            construct_spike_slots(controller + 34);
            try {
                controller[37] = 0;
                reset_police_chase(&state<Dword>(7352496));
                reset_police_chase(&state<Dword>(7352612));
            } catch (...) {
                destroy_spike_slots(controller + 34);
                throw;
            }
        } catch (...) {
            destroy_roadblock_locations(bytes(controller) + 120);
            throw;
        }
    } catch (...) {
        destroy_traffic_locations(bytes(controller) + 92);
        throw;
    }
    return controller;
}

void destroy_path_controller(Dword *controller) {
    for (unsigned offset : {80u, 86u, 84u, 82u})
        at<std::uint16_t>(controller, offset) = 0;
    controller[37] = 0;
    try {
        try {
            destroy_spike_slots(controller + 34);
        } catch (...) {
            destroy_roadblock_locations(bytes(controller) + 120);
            throw;
        }
        destroy_roadblock_locations(bytes(controller) + 120);
    } catch (...) {
        destroy_traffic_locations(bytes(controller) + 92);
        throw;
    }
    destroy_traffic_locations(bytes(controller) + 92);
}
void destroy_police_chase(void *);

void *__fastcall delete_police_chase(void *chase, void *, Dword flags) {
    destroy_police_chase(chase);
    if (flags & 1)
        gui::application_release(chase);
    return chase;
}

const Dword police_chase_methods[] = {reinterpret_cast<Dword>(delete_police_chase)};

void *construct_police_chase(void *chase) {
    for (unsigned offset : {8u, 10u, 12u})
        at<std::uint16_t>(chase, offset) = 0;
    at<Byte>(chase, 20) = 0;
    at<Dword>(chase, 16) = Dword(-1);
    for (unsigned offset : {24u, 40u, 44u, 48u, 52u, 56u, 60u, 68u, 72u})
        at<Dword>(chase, offset) = 0;
    at<Dword>(chase, 76) = Dword(-1);
    at<Dword>(chase, 80) = 0;
    at<Dword>(chase, 84) = population_frame::chase_transition_ticks(chase_transition_seconds);
    at<Dword>(chase, 88) = at<Dword>(chase, 92) = 0;
    at<Byte>(chase, 108) = 0;
    at<const Dword *>(chase, 0) = police_chase_methods;
    at<Dword>(chase, 96) = Dword(-1);
    at<Dword>(chase, 72) = at<Dword>(chase, 68);
    at<Dword>(chase, 76) = Dword(-1);
    at<Dword>(chase, 52) = at<Dword>(chase, 60) = at<Dword>(chase, 80) = 0;
    at<Dword>(chase, 56) = 5;
    at<Dword>(chase, 100) = at<Dword>(chase, 104) = 0;
    return chase;
}

void destroy_police_chase(void *chase) {
    at<const Dword *>(chase, 0) = police_chase_methods;
    reset_police_chase(chase);
}

void finalize_police_chases() {
    destroy_police_chase(&state<Dword>(7352612));
    destroy_police_chase(&state<Dword>(7352496));
}

void initialize_police_chases() {
    construct_police_chase(&state<Dword>(7352496));
    try {
        construct_police_chase(&state<Dword>(7352612));
    } catch (...) {
        destroy_police_chase(&state<Dword>(7352496));
        throw;
    }
    std::atexit(finalize_police_chases);
}

void initialize_minimum_backup_wait() {
    state<Dword>(7352472) = 600;
}

void initialize_maximum_backup_wait() {
    state<Dword>(7352468) = 900;
}

void initialize_pursuit_path_deadline() {
    state<Dword>(7352464) = 1800;
}

void initialize_helicopter_flight_limit() {
    state<Dword>(7352836) = 3600;
}

void initialize_projectile_drop_minimum() {
    state<Dword>(7352832) = 180;
}

void initialize_projectile_drop_maximum() {
    state<Dword>(7352828) = 600;
}

void initialize_helicopter_pattern_minimum() {
    state<Dword>(7352824) = 300;
}

void initialize_helicopter_pattern_maximum() {
    state<Dword>(7352820) = 300;
}

void initialize_helicopter_attachment_offset() {
    population_frame::initialize_helicopter_attachment_offset();
}

void initialize_helicopter_projectile_offset() {
    population_frame::initialize_helicopter_projectile_offset();
}

void initialize_projectile_lifetime() {
    state<Dword>(7352864) = 600;
}

void initialize_cop_tactic_interval() {
    state<Dword>(7352944) = 300;
}

void finalize_path_controller() {
    Byte &guard = state<Byte>(7353152);
    if (!(guard & 1)) {
        guard |= 1;
        destroy_path_controller(ai::pending::path_controller);
    }
}

void initialize_path_controller() {
    construct_path_controller(ai::pending::path_controller);
    std::atexit(finalize_path_controller);
}

void initialize_profile_count() {
    state<Dword>(7353148) = 0;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
 
NFS_KERNEL_INITIALIZER(startup_initialize_chase_warmup, ".CRT$XCU_NFS_0069017C",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_chase_warmup);
NFS_KERNEL_INITIALIZER(startup_initialize_helicopter_activation_delay, ".CRT$XCU_NFS_00690180",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_helicopter_activation_delay);
NFS_KERNEL_INITIALIZER(startup_initialize_threat_memory, ".CRT$XCU_NFS_00690184",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_threat_memory);
NFS_KERNEL_INITIALIZER(startup_initialize_bust_contact_grace, ".CRT$XCU_NFS_00690188",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_bust_contact_grace);
NFS_KERNEL_INITIALIZER(startup_initialize_cop_maximum_stun, ".CRT$XCU_NFS_0069018C",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_cop_maximum_stun);
NFS_KERNEL_INITIALIZER(startup_initialize_cop_recovery_delay, ".CRT$XCU_NFS_00690190",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_cop_recovery_delay);
NFS_KERNEL_INITIALIZER(startup_initialize_cop_chase_delay, ".CRT$XCU_NFS_00690194",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_cop_chase_delay);
NFS_KERNEL_INITIALIZER(startup_initialize_cop_boredom_delay, ".CRT$XCU_NFS_00690198",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_cop_boredom_delay);
NFS_KERNEL_INITIALIZER(startup_initialize_path_controller_pointer, ".CRT$XCU_NFS_0069019C",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_path_controller_pointer);
NFS_KERNEL_INITIALIZER(startup_initialize_police_chases, ".CRT$XCU_NFS_006901A0",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_police_chases);
NFS_KERNEL_INITIALIZER(startup_initialize_minimum_backup_wait, ".CRT$XCU_NFS_006901A4",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_minimum_backup_wait);
NFS_KERNEL_INITIALIZER(startup_initialize_maximum_backup_wait, ".CRT$XCU_NFS_006901A8",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_maximum_backup_wait);
NFS_KERNEL_INITIALIZER(startup_initialize_pursuit_path_deadline, ".CRT$XCU_NFS_006901AC",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_pursuit_path_deadline);
NFS_KERNEL_INITIALIZER(startup_initialize_helicopter_flight_limit, ".CRT$XCU_NFS_006901B0",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_helicopter_flight_limit);
NFS_KERNEL_INITIALIZER(startup_initialize_projectile_drop_minimum, ".CRT$XCU_NFS_006901B4",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_projectile_drop_minimum);
NFS_KERNEL_INITIALIZER(startup_initialize_projectile_drop_maximum, ".CRT$XCU_NFS_006901B8",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_projectile_drop_maximum);
NFS_KERNEL_INITIALIZER(startup_initialize_helicopter_pattern_minimum, ".CRT$XCU_NFS_006901BC",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_helicopter_pattern_minimum);
NFS_KERNEL_INITIALIZER(startup_initialize_helicopter_pattern_maximum, ".CRT$XCU_NFS_006901C0",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_helicopter_pattern_maximum);
NFS_KERNEL_INITIALIZER(startup_initialize_helicopter_attachment_offset, ".CRT$XCU_NFS_006901C4",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_helicopter_attachment_offset);
NFS_KERNEL_INITIALIZER(startup_initialize_helicopter_projectile_offset, ".CRT$XCU_NFS_006901C8",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_helicopter_projectile_offset);
NFS_KERNEL_INITIALIZER(startup_initialize_projectile_lifetime, ".CRT$XCU_NFS_006901CC",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_projectile_lifetime);
NFS_KERNEL_INITIALIZER(startup_initialize_cop_tactic_interval, ".CRT$XCU_NFS_006901D0",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_cop_tactic_interval);
NFS_KERNEL_INITIALIZER(startup_initialize_path_controller, ".CRT$XCU_NFS_006901D4",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_path_controller);
NFS_KERNEL_INITIALIZER(startup_initialize_profile_count, ".CRT$XCU_NFS_006901D8",
                       nfs::game::runtime::ai::lifecycle::pending::initialize_profile_count);
