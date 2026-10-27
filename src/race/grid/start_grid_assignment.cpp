#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../ai/population/ai_population_runtime.hpp"
#include "../../network/vehicles/remote_car_runtime.hpp"
#include <cstring>
#include <utility>

namespace nfs::game::world::pending {
namespace {
const volatile float reflected_axis_scale = -1.0f;

__declspec(noinline) long double start_marker_depth(const float *position, const float *axis) {
    return runtime::hardware::dot_product(position, axis);
}

__declspec(noinline) long double squared_start_distance(const float *first, const float *second) {
    return runtime::hardware::distance_squared(first, second);
}

void copy_start_position(void *destination, const void *source) {
    at<Dword>(destination, 0) = at<Dword>(source, 0);
    at<Dword>(destination, 4) = at<Dword>(source, 4);
    at<Dword>(destination, 8) = at<Dword>(source, 8);
}
void *start_participant(void *rules, Dword ordinal) {
    return ordinal < 24 ? offset(rules, 248 + ordinal * 84) : nullptr;
}
float *start_position(void *owner, Dword slot) {
    return reinterpret_cast<float *>(offset(owner, 700 + slot * 12));
}
float *start_rotation(void *owner, Dword slot) {
    return reinterpret_cast<float *>(offset(owner, 988 + slot * 36));
}
} // namespace


// Initialize all 24 participant override flags to zero.
 
 
void assign_start_grid(void *owner) {
    namespace population = runtime::ai::lifecycle::pending;

    Byte assigned[24]{};
    Byte override_flags[24]{}; // Explicit accepted repair described above.
    for (Dword slot = 0; slot < 24; ++slot) {
        copy_start_position(start_position(owner, slot), runtime::saved::zero_vector);
        std::memcpy(start_rotation(owner, slot), runtime::saved::identity_matrix, 36);
        at<Byte>(owner, 1852 + slot) = 1;
    }

    int start_count = at<int>(subsystem(24), 60);
    if (start_count > 24)
        start_count = 24;
    if (start_count != 0) {
        const auto *first_marker = static_cast<const float *>(
            runtime::level_marker(static_cast<Dword *>(subsystem(24)), 0, 0));
        float first_rotation[9];
        quaternion_matrix(first_marker + 3, first_rotation);
        float grid_axis[3];
        grid_axis[0] = -first_rotation[6];
        grid_axis[1] = -first_rotation[7];
        at<Dword>(grid_axis, 8) = at<Dword>(first_rotation, 32);
        float grid_depth[24];
        int grid_order[24];
        for (int marker = 0; marker < start_count; ++marker) {
            const auto *position = static_cast<const float *>(
                runtime::level_marker(static_cast<Dword *>(subsystem(24)), 0, marker));
            grid_order[marker] = marker;
            grid_depth[marker] = float(start_marker_depth(position, grid_axis));
        }
        for (int sorted = 1; sorted < start_count; ++sorted) {
            int current = sorted;
            while (current > 0 && grid_depth[current] < grid_depth[current - 1]) {
                std::swap(grid_depth[current], grid_depth[current - 1]);
                std::swap(grid_order[current], grid_order[current - 1]);
                --current;
            }
        }

        void *rules = subsystem(72);
        int background_racers = 0;
        Byte has_human_cop = 0;
        Byte has_human_racer = 0;
        for (Dword ordinal = 0; ordinal < at<Dword>(rules, 168); ++ordinal) {
            void *participant = start_participant(rules, ordinal);
            if (at<Dword>(participant, 0)) {
                if (at<Dword>(participant, 8))
                    has_human_cop = 1;
                else
                    has_human_racer = 1;
            } else if (!at<Dword>(participant, 8)) {
                ++background_racers;
            }
        }
        const Byte use_cop_starts = Byte(has_human_cop && !has_human_racer);

        const float *cop_starts[2];
        void *cop_sections[2];
        int cop_nodes[2];
        for (unsigned player = 0; player < 2; ++player) {
            const auto *point =
                reinterpret_cast<const float *>(offset(subsystem(36), 24 + player * 12));
            cop_starts[player] = point;
            cop_sections[player] = nullptr;
            // TEST AH,44h parity accepts nonzero and unordered components.
            override_flags[player] = Byte(point[0] != 0 || point[1] != 0 || point[2] != 0);
            cop_nodes[player] = -1;
        }

        int normal_slots[24];
        int normal_count = 0;
        int cop_start_count = 0;
        for (Dword ordinal = 0; ordinal < at<Dword>(rules, 168); ++ordinal) {
            void *participant = start_participant(rules, ordinal);
            const bool human = at<Dword>(participant, 0) != 0;

            if (use_cop_starts && human && override_flags[ordinal]) {
                ++cop_start_count;
            } else if (human || (!at<Dword>(participant, 8) && !has_human_cop)) {
                normal_slots[normal_count++] = at<int>(participant, 20);
            }
        }
        for (int sorted = 1; sorted < normal_count; ++sorted) {
            int current = sorted;
            while (current > 0 && normal_slots[current] < normal_slots[current - 1]) {
                std::swap(normal_slots[current], normal_slots[current - 1]);
                --current;
            }
        }

        if (use_cop_starts) {
            for (Dword ordinal = 0; ordinal < at<Dword>(rules, 168); ++ordinal) {
                void *participant = start_participant(rules, ordinal);
                const Dword slot = at<Dword>(participant, 20);

                if (at<Dword>(participant, 0) && override_flags[ordinal]) {
                    const float *point = cop_starts[ordinal];
                    copy_start_position(start_position(owner, slot), point);
                    float *rotation = start_rotation(owner, slot);
                    std::memcpy(rotation, runtime::saved::identity_matrix, 36);
                    float query[3];
                    copy_start_position(query, point);
                    at<volatile float>(query, 8) = -at<volatile float>(query, 8);
                    float projected_position[3];
                    population::project_reset_position(subsystem(76), query,
                                                       &cop_sections[ordinal], &cop_nodes[ordinal],
                                                       projected_position, rotation, 0, nullptr);
                    assigned[slot] = 1;
                }
            }
        }

        const int course_points = population::track_point_count(runtime::track_collision);
        int course_indices[2] = {0, 0};
        Byte have_path_pose[2] = {0, 0};
        int course_spacing[2];
        course_spacing[0] = course_spacing[1] =
            background_racers ? course_points / background_racers : 0;
        float path_positions[2][3];
        float path_rotations[2][9];
        if (override_flags[0] && override_flags[1]) {
            for (unsigned player = 0; player < 2; ++player) {
                const float *point = cop_starts[player];
                float query[3];
                copy_start_position(query, point);
                at<volatile float>(query, 8) = -at<volatile float>(query, 8);
                course_indices[player] =
                    population::nearest_track_point(runtime::track_collision, query);
                void *section = cop_sections[player];
                int next_node;
                bool used_section = false;
                if (section && cop_nodes[player] >= 0) {
                    next_node = cop_nodes[player] + 1;
                    while (next_node < at<int>(section, 36)) {
                        const auto *sample = static_cast<const float *>(
                            population::section_position(section, next_node));
                        float candidate[3];
                        copy_start_position(candidate, sample);
                        at<volatile float>(candidate, 8) = -at<volatile float>(candidate, 8);
                        const auto distance = squared_start_distance(candidate, point);
                        if (distance > 900.0f) {
                            population::sample_route_pose(section, next_node,
                                                          path_positions[player],
                                                          path_rotations[player], 0);
                            have_path_pose[player] = 1;
                            break;
                        }
                        ++next_node;
                    }
                    if (next_node < at<int>(section, 36)) {
                        const auto *sample = static_cast<const float *>(
                            population::section_position(section, next_node));
                        course_indices[player] =
                            population::nearest_track_point(runtime::track_collision, sample);
                        used_section = true;
                    }
                }
                if (!used_section) {
                    const int search_limit = course_points / 4;
                    for (int visited = 0; visited < search_limit; ++visited) {
                        ++course_indices[player];
                        if (course_indices[player] >= course_points)
                            course_indices[player] -= course_points;
                        float candidate[3], rotation[9];
                        runtime::physics::sample_track_reset_pose(
                            runtime::track_collision, course_indices[player], candidate, rotation);
                        const auto distance = squared_start_distance(candidate, point);
                        if (distance > 900.0f)
                            break;
                    }
                }
            }
            if (cop_start_count == 2) {
                int distance = int(Dword(course_indices[1]) - Dword(course_indices[0]));
                if (distance < 0)
                    distance = int(Dword(distance) + Dword(course_points));
                course_spacing[0] = distance / (background_racers / 2);
                distance = int(Dword(course_indices[0]) - Dword(course_indices[1]));
                if (distance < 0)
                    distance = int(Dword(distance) + Dword(course_points));
                course_spacing[1] = distance / ((background_racers + 1) / 2);
            }
        }

        int background_assigned = 0;
        audio::vehicle::Progress progress;
        audio::vehicle::construct_progress(&progress, at<int>(rules, 168));
        try {
            for (Dword ordinal = 0; ordinal < at<Dword>(rules, 168); ++ordinal) {
                audio::vehicle::advance_progress(&progress);
                void *participant = start_participant(rules, ordinal);
                const Dword slot = at<Dword>(participant, 20);
                const bool human = at<Dword>(participant, 0) != 0;

                if (use_cop_starts && human && override_flags[ordinal])
                    continue;
                if (!human && (at<Dword>(participant, 8) || has_human_cop)) {
                    if (at<Dword>(participant, 8))
                        continue;
                    unsigned start = 0;
                    if (cop_start_count > 1 && background_assigned >= background_racers / 2)
                        start = 1;
                    float *position = start_position(owner, slot);
                    float *rotation = start_rotation(owner, slot);
                    runtime::physics::sample_track_reset_pose(
                        runtime::track_collision, course_indices[start], position, rotation);
                    if ((background_assigned == 0 && have_path_pose[0]) ||
                        (background_assigned == background_racers / 2 && have_path_pose[1])) {
                        copy_start_position(position, path_positions[start]);
                        std::memcpy(rotation, path_rotations[start], 36);
                    }
                    int next = int(Dword(course_indices[start]) + Dword(course_spacing[start]));
                    if (next >= course_points)
                        next = int(Dword(next) - Dword(course_points));
                    course_indices[start] = next;
                    at<Byte>(owner, 1852 + slot) = 0;
                    ++background_assigned;
                } else {
                    int order = 0;
                    while (order < normal_count && Dword(normal_slots[order]) != slot)
                        ++order;
                    if (order >= normal_count)
                        continue;
                    const auto *marker = static_cast<const float *>(runtime::level_marker(
                        static_cast<Dword *>(subsystem(24)), 0, grid_order[order]));
                    float *rotation = start_rotation(owner, slot);
                    quaternion_matrix(marker + 3, rotation);
                    rotation[2] = float(static_cast<long double>(rotation[2]) * reflected_axis_scale);
                    rotation[5] = float(static_cast<long double>(rotation[5]) * reflected_axis_scale);
                    // Keep the native position-store placement between the rotation products.
                    const Dword marker_y = at<Dword>(marker, 4);
                    const auto held_rotation_six = static_cast<long double>(rotation[6]);
                    float *position = start_position(owner, slot);
                    at<Dword>(position, 0) = at<Dword>(marker, 0);
                    const auto rotation_six = held_rotation_six * reflected_axis_scale;
                    const Dword marker_z = at<Dword>(marker, 8);
                    at<Dword>(position, 4) = marker_y;
                    at<Dword>(position, 8) = marker_z;
                    rotation[6] = float(rotation_six);
                    rotation[7] = float(static_cast<long double>(rotation[7]) * reflected_axis_scale);
                }
                assigned[slot] = 1;
            }
        } catch (...) {
            audio::vehicle::destroy_progress(&progress);
            throw;
        }
        audio::vehicle::destroy_progress(&progress);
    }

    int missing_positions = 1;
    for (Dword slot = 1; slot < 24; ++slot) {
        if (assigned[slot]) {
            missing_positions = 1;
            continue;
        }
        float *position = start_position(owner, slot);
        const auto missing_count = static_cast<long double>(missing_positions);
        copy_start_position(position, start_position(owner, slot - 1));
        const auto displacement = missing_count + missing_count;
        position[0] = float(displacement + position[0]);
        std::memcpy(start_rotation(owner, slot), start_rotation(owner, slot - 1), 36);
        position[2] = float(displacement + position[2]);
        ++missing_positions;
    }
    if (network::session_active() &&
        (at<std::int16_t>(subsystem(104), 10) < 4 || at<int>(subsystem(72), 168) < 1)) {
        const void *marker = runtime::level_marker(static_cast<Dword *>(subsystem(24)), 0, 7);
        copy_start_position(offset(owner, 784), marker);
    }
}
} // namespace nfs::game::world::pending
