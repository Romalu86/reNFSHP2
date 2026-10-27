#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"

namespace nfs::game::runtime::ai::lifecycle::pending {
// Compatibility names from the earlier frontier; the providers use the observed skin/model roles.
int parse_car_kind(const char *name) {
    return dependencies::parse_skin_variant(name);
}
int parse_car_variant(int value) {
    return dependencies::validate_skin_variant(value);
}
Dword car_kind_name(Dword model) {
    return reinterpret_cast<Dword>(dependencies::car_model_name(model));
}
int parse_car_skin(const char *name) {
    return dependencies::parse_car_model(name);
}
Dword select_car_kind(int model) {
    return dependencies::validate_car_model(model);
}
void synchronize_track_vehicle(void *car) {
    dependencies::synchronize_track_vehicle(car);
}
void attach_pursuit_car(Dword *owner, Dword *car) {
    dependencies::attach_pursuit_car(owner, car);
}
void sample_section_pose(void *section, int index, float *pose) {
    dependencies::sample_section_pose(section, index, pose);
}
void sample_route_pose(void *section, int index, float *position, float *orientation,
                       Byte reverse) {
    dependencies::sample_route_pose(section, index, position, orientation, reverse);
}
void *enumerate_path_sections(void *reference, Dword include_previous) {
    return dependencies::enumerate_path_sections(reference, include_previous);
}
void set_participant_enabled(void *owner, Dword slot, int enabled) {
    dependencies::set_participant_enabled(owner, slot, static_cast<Byte>(enabled));
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
