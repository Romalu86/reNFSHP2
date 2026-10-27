#pragma once
#include <cstdint>
namespace nfs::game::runtime::ai::cars::path_frame {
void steering_native80(void *, const float *, void *, void *);
void set_target_speed(void *, std::uint32_t, std::uint32_t);
void recover_motion(void *, std::uint32_t);
namespace legacy { int truncate_control(const void *); }
}
namespace nfs::game::runtime::ai::entities::pending::path_frame {
void horizontal_distance_native80(const float *, const float *, void *);
void advance_distance_native80(void *, float, int *, std::uint32_t *, void *);
long double advance_distance(void *, float, int *, std::uint32_t *);
void discard_advance_distance(void *, float, int *, std::uint32_t *);
std::uint32_t sample_target(void *, float *, float *, std::uint32_t *, float *, float,
                            const float *, std::uint8_t);
void move_entity(void *, float, const float *);
void brake_entity(void *, std::uint32_t *);
void select_helicopter_destination(void *);
}
namespace nfs::game::runtime::ai::entities::pending {
extern float minimum_path_width;
}
namespace nfs::game::runtime::ai::lifecycle::pending::path_frame {
void node_distance_native80(const float *, const float *, void *);
void discard_node_distance(const float *, const float *);
void closest_sample(void *, const float *, int *, float *);
int nearest_node(void *, const float *);
std::uint8_t point_in_quad(const float *, const float *, const float *, const float *,
                           const float *);
void *containing_section(void *, const float *);
void construct_route(void *, void *, std::uint32_t *, std::uint32_t *, const float *,
                      const float *, void *, std::uint32_t, std::uint32_t);
bool recovery_path_moved(const float *, const float *);
}
