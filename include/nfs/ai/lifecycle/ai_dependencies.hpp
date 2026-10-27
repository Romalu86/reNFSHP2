#pragma once
#include "nfs/physics/solver/solver_runtime.hpp"

namespace nfs::game::runtime::ai::dependencies {
struct SkinName {
    int variant;
    const char *name;
};
struct CarName {
    int model;
    Dword flags;
    const char *name;
};
static_assert(sizeof(SkinName) == 8 && sizeof(CarName) == 12);
extern const SkinName skin_names[19];
extern const CarName car_names[39];
int parse_skin_variant(const char *);
int validate_skin_variant(int);
const char *car_model_name(Dword);
int parse_car_model(const char *);
Dword validate_car_model(int);
void reset_path_reservations(void *);
void release_contact_registry(Dword *);
void attach_pursuit_car(Dword *, Dword *);
void synchronize_track_vehicle(void *);
void sample_section_tangent(void *, int, float *);
void sample_horizontal_tangent(void *, int, float *);
void sample_section_pose(void *, int, float *);
void reverse_section_pose(float *);
void sample_route_pose(void *, int, float *, float *, Byte);
void *section_last_node(void *);
void *section_node(void *, int);
void *path_group_entry(void *, int);
Dword section_candidate(void *, std::uint16_t);
void append_section_candidate(void *, Dword);
void *enumerate_path_sections(void *, Dword);
void set_participant_enabled(void *, Dword, Byte);
} // namespace nfs::game::runtime::ai::dependencies
