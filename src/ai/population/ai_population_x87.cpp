#include "ai_population_runtime.hpp"
#include "ai_population_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cfloat>
#include <cstring>
namespace nfs::game::runtime::ai::lifecycle::pending {
extern Dword traffic_reservation_queries;
void closest_section_sample(void *, const float *, int *, float *);
void *first_path_node(void *);
void *construct_path_vertex(void *, const float *);
void *section_position(void *, int);
}
namespace nfs::game::runtime::ai::lifecycle::pending::population_frame {
namespace {
inline long double number(float value) { return static_cast<long double>(value); }
inline long double point_distance(const float *first, const float *second) {
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(second[axis]) - first[axis]);
    return hardware::vector_length(delta);
}
void *entity_interface(void *entity, unsigned slot) {
    void *base = bytes(entity) + 4;
    return bytes(base) + reinterpret_cast<const int *>(at<Dword>(base, 0))[slot];
}
void *car_interface(void *car, unsigned slot) { return entity_interface(car, slot); }

bool circuit_track() {
    Dword *tracks = system(40);
    return at<Byte>(at<void *>(tracks, 12 + tracks[2] * 4), 24) != 0;
}

bool retain_location(void *selected, Byte selected_near, int selected_distance, Byte within_range,
                     int distance) {
    if (!selected)
        return true;
    if (!selected_near && within_range)
        return false;
    if (selected_near && !within_range)
        return true;
    return distance < selected_distance;
}

void *graph_group(void *graph, Dword index) {
    return bytes(graph) + 4 + index * 52;
}

float &group_distance(void *graph, Dword index) {
    return at<float>(graph, 52 + index * 52);
}

void *group_section(void *group, Dword ordinal) {
    return reinterpret_cast<void *>(
        dependencies::section_candidate(bytes(group) + 12, std::uint16_t(ordinal)));
}

Dword collect_group_sections(void *group, Dword endpoint, Dword group_index, void **sections) {
    Dword count = 0;
    for (Dword i = 0; i < at<std::uint16_t>(group, 44); ++i) {
        void *section = group_section(group, i);
        if (at<Dword>(section, endpoint) == group_index && count < 8)
            sections[count++] = section;
    }
    return count;
}

bool propagate_group_distances(void *graph, bool subtract_length) {
    bool changed = false;
    for (Dword i = 0; i < at<std::uint16_t>(graph, 16858); ++i) {
        if (group_distance(graph, i) != -1.f)
            continue;
        void *connections[8];
        const Dword count =
            collect_group_sections(graph_group(graph, i), subtract_length ? 24 : 20, i,
                                   connections);
        for (Dword j = 0; j < count; ++j) {
            void *section = connections[j];
            const float other =
                group_distance(graph, at<Dword>(section, subtract_length ? 20 : 24));
            if (other != -1.f) {
                const auto section_length =
                    static_cast<long double>(at<float>(section, 44));
                group_distance(graph, i) =
                    float(subtract_length ? static_cast<long double>(other) - section_length
                                          : section_length + static_cast<long double>(other));
                changed = true;
                break;
            }
        }
    }
    return changed;
}

bool coincident_endpoint(const float *first, const float *second) {
    for (unsigned axis = 0; axis < 3; ++axis) {
        auto difference = static_cast<long double>(first[axis]) - second[axis];
        if (difference < static_cast<long double>(0))
            difference = -difference;
        if (!(difference < static_cast<long double>(1)))
            return false;
    }
    return true;
}

} // namespace

void bind_location_path(void *location, void *reference, const float *position, void *path) {
    float direction[] = {1, 0, 0};
    float pose[21], orientation[4];
    construct_pursuit_route(bytes(system(76)) + 4, reference, &at<Dword>(location, 56),
                            reinterpret_cast<Dword *>(orientation), direction, position, path, 0,
                            0);
    const int node = at<int>(location, 56);
    if (node >= 0) {
        void *section = at<void *>(reference, 8);
        at<void *>(location, 52) = section;
        if (section) {
            void *point = dependencies::section_node(section, node);
            const long double left_width = at<float>(point, 12);
            const long double right_width = at<float>(point, 16);
            const float threshold = float((right_width + left_width) * 0.5f);
            sample_section_pose(at<void *>(location, 52), node, pose);
            float delta[3];
            for (int i = 0; i < 3; ++i)
                delta[i] = float(static_cast<long double>(position[i]) - static_cast<long double>(pose[i]));
            if (hardware::dot_product(pose + 9, delta) < static_cast<long double>(threshold))
                at<Byte>(location, 60) = 1;
        }
    }
}

void anchor_paths(Dword *manager) {
    float position[3];
    if (at<Dword>(system(24), 80)) {
        const float *start = static_cast<float *>(level_marker(system(24), 5, 0));
        position[0] = start[0];
        position[1] = start[1];
        position[2] = -start[2];
        anchor_path_graph(manager + 1, position);
        return;
    }
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    void *selected = nullptr;
    float selected_projection;
    for (Dword word = network::world::first(&iterator); word;
         word = network::world::next(&iterator)) {
        void *entity = reinterpret_cast<void *>(word);
        if (at<Byte>(entity, 212) && invoke<Dword>(entity_interface(entity, 2), 2)) {
            void *actor = invoke<void *>(entity_interface(entity, 2), 1);
            invoke(actor, 31);
            float bounds[6];
            invoke(entity_interface(actor, 3), 2, bounds);
            const auto projected = hardware::dot_product(bounds, invoke<const float *>(actor, 0));
            if (!selected ||
                static_cast<long double>(selected_projection) <
                    projected) {
                selected_projection = float(projected);
                selected = actor;
            }
        }
    }
    const float zero[] = {0, 0, 0};
    const float *chosen = selected ? invoke<const float *>(selected, 0) : zero;
    std::memcpy(position, chosen, sizeof(position));
    if (selected) {
        void *section = invoke<void *>(selected, 1);
        int node = invoke<int>(selected, 2);
        if (section && node >= 0) {
            if (node < int(at<Dword>(section, 36) - 1))
                ++node;
            std::memcpy(position, section_position(section, node), sizeof(position));
        }
    }
    anchor_path_graph(manager + 1, position);
}

Dword reserve_traffic_location(void *owner, Dword requester) {
    if (at<Dword>(owner, 20) <= at<Dword>(owner, 24) || !at<Dword>(owner, 16))
        return 0;
    void *selected = nullptr;
    Byte selected_near = 0;
    int selected_distance = -1;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    const bool circuit = circuit_track();
    ++traffic_reservation_queries;
    Dword entity_word = network::world::first(&iterator);
    while (!network::world::iterator_done(&iterator)) {
        void *entity = reinterpret_cast<void *>(entity_word);
        void *car = invoke<void *>(car_interface(entity, 2), 1);
        if (car && invoke<Byte>(car, 13)) {
            const float *first = path_position(bytes(car) + 220, 0);
            const float *next = path_position(bytes(car) + 220, 1);
            void *state_owner = invoke<void *>(car_interface(entity, 2), 3);
            if (!at<Dword>(state_owner, 8)) {
                const int current = at<int>(at<void *>(car, 368), 7984);
                void *population = system(76);
                Dword count = at<Dword>(owner, 8);
                Byte *location = bytes(at<void *>(owner, 12));
                while (count) {
                    if (current <= at<int>(location, 8) || circuit) {
                        int distance = int(at<Dword>(location, 8) - Dword(current));
                        if (distance < 0)
                            distance = int(Dword(distance) +
                                           Dword(track_point_count(game::runtime::track_collision)));
                        if (location_available(location)) {
                            float position[3];
                            std::memcpy(position, bytes(location) + 16, sizeof(position));
                            position[2] = float(-static_cast<long double>(position[2]));
                            bool ahead = true;
                            if (current == at<int>(location, 8)) {
                                const float next_distance = float(point_distance(position, next));
                                ahead = static_cast<long double>(next_distance) <
                                        point_distance(position, first);
                            }
                            if (ahead) {
                                const Byte within_range =
                                    player_within_range(population, position, spawn_distance);
                                if (retain_location(selected, selected_near, selected_distance,
                                                    within_range, distance)) {
                                    selected = location;
                                    selected_near = within_range;
                                    selected_distance = distance;
                                    at<Dword>(location, 36) =
                                        invoke<Dword>(car_interface(entity, 2), 2);
                                }
                            }
                        }
                    }
                    location += 40;
                    --count;
                }
                if (selected && !selected_near)
                    break;
            }
        }
        entity_word = network::world::next(&iterator);
    }
    if (selected) {
        void *state_owner = car_interface(at<void *>(selected, 36), 3);
        void *body = invoke<void *>(state_owner, 0);
        if (body) {
            at<Dword>(state_owner, 4) = at<Dword>(body, 680);
            at<Dword>(state_owner, 8) = at<Dword>(body, 684);
            at<float>(state_owner, 12) = float(-static_cast<long double>(at<float>(body, 688)));
        } else
            at<Dword>(state_owner, 4) = at<Dword>(state_owner, 8) = at<Dword>(state_owner, 12) = 0;
        --at<Dword>(owner, 16);
        at<Dword>(selected, 28) = requester;
        ++at<Dword>(owner, 24);
    }
    return reinterpret_cast<Dword>(selected);
}

void recount_entity_kinds(Dword *owner) {
    at<Dword *>(owner, 4) = state<Dword *>(7353312);
    at<Dword>(owner, 8) = 0;
    for (unsigned kind = 0; kind < 10; ++kind) {
        at<Dword>(owner, 12 + kind * 12) = 0;
        at<Dword>(owner, 16 + kind * 12) = 30;
        at<Dword>(owner, 20 + kind * 12) = 0;
    }
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, at<Dword *>(owner, 4));
    Dword entity = network::world::first(&iterator);
    int previous_kind = 10;
    while (!network::world::iterator_done(&iterator)) {
        void *vehicle = reinterpret_cast<void *>(entity);
        if (ai::kind(vehicle) != previous_kind) {
            previous_kind = ai::kind(vehicle);
            at<Dword>(owner, (previous_kind * 3 + 3) * 4) = 0;
        }
        ++at<Dword>(owner, (previous_kind * 3 + 3) * 4);
        entity = network::world::next(&iterator);
    }
    for (unsigned kind = 0; kind < 10; ++kind) {
        const Dword count = at<Dword>(owner, 12 + kind * 12);
        if (!count) {
            at<Dword>(owner, 16 + kind * 12) = at<Dword>(owner, 20 + kind * 12) = 0;
        } else {
            const long double divisor = static_cast<long double>(30.0f) /
                                     static_cast<long double>(at<Dword>(owner, 16 + kind * 12));
            const long double population = static_cast<long double>(count);
            at<Dword>(owner, 20 + kind * 12) =
                truncate_retained(population / (population / divisor));
        }
    }
}

int nearest_track_point(void *level, const float *position) {
    void *track = at<void *>(level, 28);
    if (!track)
        return 0;
    const int count = at<int>(track, 20);
    if (!count)
        return 0;
    const float *points = at<float *>(track, 32);
    auto distance = [&](const float *point) {
        const long double x = static_cast<long double>(position[0]) - static_cast<long double>(point[0]);
        const long double z = static_cast<long double>(position[2]) - static_cast<long double>(point[2]);
        const long double z_squared = z * z;
        const long double x_squared = x * x;
        return z_squared + x_squared;
    };
    float closest = float(distance(points));
    int selected = 0;
    for (int i = 1; i < count; ++i) {
        const auto candidate = distance(points + i * 3);
        if (candidate < static_cast<long double>(closest)) {
            closest = float(candidate);
            selected = i;
        }
    }
    return selected;
}

Byte player_within_range(void *owner, const float *point, float range) {
    const float squared_range = float(static_cast<long double>(range) * range);
    for (unsigned player = 0; player < 2; ++player) {
        void *car = at<void *>(owner, 84164 + player * 4);
        if (car && at<Byte>(car, 212)) {
            const auto distance = hardware::distance_squared(invoke<const float *>(car, 0), point);
            // Native TEST AH,41 accepts ordered <= after the squared-range float store.
            if (distance <= static_cast<long double>(squared_range))
                return 1;
        }
    }
    return 0;
}

void anchor_path_graph(void *graph, const float *position) {
    if (!at<std::uint16_t>(graph, 16856))
        return;
    for (Dword i = 0; i < at<std::uint16_t>(graph, 16858); ++i)
        group_distance(graph, i) = -1.f;
    void *selected = nullptr;
    int selected_sample = -1;
    float closest = FLT_MAX;
    const Dword sections = at<std::uint16_t>(graph, 16856);
    for (Dword i = 0; i < sections; ++i) {
        void *section = at<void *>(graph, 15656 + i * 4);
        int sample;
        float distance;
        closest_section_sample(section, position, &sample, &distance);
        if (distance < closest) {
            closest = distance;
            selected = section;
            selected_sample = sample;
        }
    }
    void *node = dependencies::section_node(selected, selected_sample);
    float pose[21], delta[3];
    sample_section_pose(selected, selected_sample, pose);
    for (int i = 0; i < 3; ++i)
        delta[i] = float(static_cast<long double>(position[i]) - static_cast<long double>(pose[i]));
    auto distance = hardware::dot_product(delta, pose + 3) +
                    static_cast<long double>(at<float>(node, 28));
    if (distance < static_cast<long double>(0.1f))
        distance = static_cast<long double>(0.1f);
    const float maximum =
        float(static_cast<long double>(at<float>(selected, 44)) - static_cast<long double>(0.1f));
    if (static_cast<long double>(maximum) < distance)
        distance = static_cast<long double>(maximum);
    const bool inspect_connections = launch::field<int>(8627900) > 0;
    void *section = selected;
    for (;;) {
        const Dword group_index = at<Dword>(section, 20);
        const float stored_distance = float(distance);
        if (group_distance(graph, group_index) != -1.f)
            break;
        group_distance(graph, group_index) = stored_distance;
        void *group = graph_group(graph, group_index), *incoming = nullptr;
        for (Dword i = 0; i < at<std::uint16_t>(group, 44); ++i) {
            void *candidate = group_section(group, i);
            if (at<Dword>(candidate, 24) == group_index &&
                (!incoming || at<int>(candidate, 32) < at<int>(incoming, 32)))
                incoming = candidate;
        }
        if (!incoming)
            break;
        section = incoming;
        distance = static_cast<long double>(at<float>(section, 44)) + static_cast<long double>(stored_distance);
    }
    while (propagate_group_distances(graph, false)) {
    }
    while (propagate_group_distances(graph, true)) {
    }
    if (inspect_connections)
        for (Dword i = 0; i < at<std::uint16_t>(graph, 16858); ++i) {
            // The release image builds these two local lists without emitting diagnostics.
            void *connections[8];
            void *group = graph_group(graph, i);
            collect_group_sections(group, 20, i, connections);
            collect_group_sections(group, 24, i, connections);
        }
    Dword *tracks = system(40);
    if (at<Byte>(at<void *>(tracks, 12 + tracks[2] * 4), 24)) {
        const float end = group_distance(graph, at<Dword>(selected, 24));
        at<float>(graph, 16864) =
            end > 0 ? float((static_cast<long double>(at<float>(selected, 44)) + static_cast<long double>(end)) -
                            static_cast<long double>(group_distance(graph, at<Dword>(selected, 20))))
                    : -1.f;
    } else {
        float maximum_distance = 0;
        for (Dword i = 0; i < at<std::uint16_t>(graph, 16858); ++i)
            if (maximum_distance < group_distance(graph, i))
                maximum_distance = group_distance(graph, i);
        at<float>(graph, 16864) = maximum_distance;
    }
}

void build_path_arc_distances(void *section) {
    int remaining = at<int>(section, 36);
    auto *node = at<float *>(section, 40);
    at<Dword>(node, 28) = 0;
    float distance = 0;
    if (remaining > 1) {
        --remaining;
        do {
            distance = float(point_distance(node + 8, node) + distance);
            at<float>(node, 60) = distance;
            node += 8;
        } while (--remaining);
        at<float>(section, 44) = distance;
    } else
        at<Dword>(section, 44) = 0;
}

Byte insert_path_section(void *group, void *section) {
    if (at<int>(section, 36) < 2)
        return 0;
    const auto *start = static_cast<const float *>(first_path_node(section));
    const auto *finish = static_cast<const float *>(dependencies::section_last_node(section));
    build_path_arc_distances(section);
    auto &sections = at<std::uint16_t>(group, 16856);
    at<void *>(group, 15656 + sections * 4) = section;
    ++sections;
    auto &vertices = at<std::uint16_t>(group, 16858);
    const float *endpoints[] = {start, finish};
    for (unsigned end = 0; end < 2; ++end) {
        float position[3];
        std::memcpy(position, endpoints[end], 12);
        Dword index = 0;
        while (index < vertices &&
               !coincident_endpoint(reinterpret_cast<float *>(bytes(group) + 4 + index * 52),
                                    position))
            ++index;
        if (index == vertices) {
            ++vertices;
            construct_path_vertex(bytes(group) + 4 + index * 52, position);
        }
        at<Dword>(section, 20 + end * 4) = index;
    }
    for (unsigned index = 0; index < vertices; ++index) {
        void *vertex = bytes(group) + 4 + index * 52;
        if (coincident_endpoint(static_cast<const float *>(vertex), start) ||
            coincident_endpoint(static_cast<const float *>(vertex), finish)) {
            auto &count = at<std::uint16_t>(vertex, 44);
            if (count < 8)
                at<void *>(vertex, 12 + count++ * 4) = section;
        }
    }
    at<Byte>(group, 16860) = 1;
    return 1;
}

Dword chase_transition_ticks(float seconds) {
    const auto ticks = number(seconds) * chase_transition_multiplier;
    return truncate_retained(ticks);
}
void initialize_helicopter_attachment_offset() {
    state<Dword>(7352848) = 0;
    state<float>(7352844) = float(number(helicopter_scale) * helicopter_attachment);
}
void initialize_helicopter_projectile_offset() {
    state<float>(7352852) = float(number(helicopter_scale) * helicopter_projectile_x);
    state<float>(7352856) = float(number(helicopter_scale) * helicopter_projectile_y);
    state<float>(7352860) = float(number(helicopter_scale) * helicopter_projectile_z);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending::population_frame
