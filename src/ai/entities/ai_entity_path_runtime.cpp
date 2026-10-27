#include "../driving/ai_path_frame_x87.hpp"
#include "ai_entity_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::entities {

void initialize_entity_path(void *path, void *graph, const float *heading, const float *position,
                            void *entity) {
    const Byte use_path = invoke<Dword>(entity, 9) != 0;
    Dword sample, direction;
    lifecycle::pending::construct_pursuit_route(graph, bytes(path) + 40, &sample, &direction,
                                                heading, position, path, use_path, 0);
    const Dword kind = invoke<Dword>(interface_at(entity, 1), 0);
    if (kind != 3 && kind != 7)
        direction = invoke<Dword>(entity, 28);
    at<Dword>(path, 4) = at<Dword>(path, 16);
    at<Dword>(path, 8) = at<Dword>(path, 20);
    at<Dword>(path, 12) = at<Dword>(path, 24);
    at<Dword>(path, 16) = sample;
    at<Dword>(path, 0) = 1;
    at<Dword>(path, 20) = direction;
    at<Dword>(path, 24) = at<Dword>(path, 48);
    lifecycle::pending::update_path_branch(path, entity);
}
// Complete RET4 leaf.
void finish_entity_path(void *, void *) {
}

Byte path_has_section(void *path) {
    return at<Dword>(path, 24) != 0;
}

Dword path_sample_index(void *cursor) {
    return at<Dword>(cursor, 0);
}

void __fastcall reset_path_entity(void *entity, void *) {
    reset_entity(entity, nullptr);
    at<Dword>(entity, 308) = at<Dword>(entity, 312) = at<Dword>(entity, 316) = 0;
    void *path = bytes(entity) + 220;
    lifecycle::clear_path(path);
    const float *position = invoke<const float *>(entity, 0);
    Dword *owner = system(76);
    const Dword group = invoke<Dword>(entity, 15);
    void *body = invoke<void *>(interface_at(entity, 3), 1);
    initialize_entity_path(path, bytes(owner) + 4 + group * 42080,
                           reinterpret_cast<const float *>(bytes(body) + 8), position, entity);
    finish_entity_path(path, entity);
}

void __fastcall set_path_entity_active(void *entity, void *, Dword active) {
    if (void *body = invoke<void *>(interface_at(entity, 3), 1))
        at<Byte>(body, 728) = Byte(active);
    set_entity_active(entity, nullptr, active);
}
// Code queries the virtual body twice before changing its state.
void __fastcall set_shape_entity_active(void *entity, void *, Dword active) {
    if (invoke<void *>(interface_at(entity, 3), 1))
        at<Byte>(invoke<void *>(interface_at(entity, 3), 1), 728) = Byte(active);
    set_entity_active(entity, nullptr, active);
}

void __fastcall set_spike_active(void *entity, void *, Dword active) {
    set_entity_active(entity, nullptr, at<Byte>(entity, 232) ? active : 0);
}

void __fastcall move_towards_position(void *entity, void *, float speed, const float *target) {
    pending::path_frame::move_entity(entity, speed, target);
}

Dword __fastcall choose_path_branch(void *entity, void *, void *branches) {
    const Dword count = at<std::uint16_t>(branches, 32);
    if (!count)
        return 0;
    if (count == 1)
        return at<Dword>(branches, 0);
    Dword selected = 0;
    void *section = at<void *>(entity, 244);
    const Dword endpoint =
        section ? at<Dword>(section, at<Dword>(entity, 240) == 1 ? 24 : 20) : Dword(-1);
    for (Dword i = 0; i < count; ++i) {
        void *candidate = at<void *>(branches, (i & 65535) * 4);
        if (endpoint == at<Dword>(candidate, 20) &&
            (!selected ||
             at<int>(candidate, 32) < at<int>(reinterpret_cast<void *>(selected), 32)))
            selected = reinterpret_cast<Dword>(candidate);
    }
    return selected ? selected : at<Dword>(branches, 0);
}

void __fastcall refresh_entity_route(void *entity, void *) {
    Dword *owner = system(76);
    const Dword group = invoke<Dword>(entity, 15);
    void *graph = bytes(owner) + 4 + group * 42080;
    alignas(4) Byte reference[48];
    lifecycle::reset_path_reference(reference);
    float heading[3];
    invoke(interface_at(entity, 3), 2, heading);
    Byte use_path = 1;
    if (invoke<Dword>(interface_at(entity, 1), 0) != 0 &&
        invoke<Dword>(interface_at(entity, 1), 0) != 4 &&
        invoke<Dword>(interface_at(entity, 1), 0) != 1 &&
        invoke<Dword>(interface_at(entity, 1), 0) != 3)
        use_path = 0;
    void *path = bytes(entity) + 220;
    // The call passes only the initialized low byte of the use-path stack word.
    const float *position = invoke<const float *>(entity, 0);
    Dword sample, direction;
    lifecycle::pending::construct_pursuit_route(graph, reference, &sample, &direction, heading,
                                                position, path, use_path, 0);
    const Dword override_direction = invoke<Dword>(entity, 28);
    if (override_direction)
        direction = override_direction;
    if (path_has_section(path))
        lifecycle::pending::update_path_branch(path, entity);
    void *section = at<void *>(reference, 8);
    if (section && (section != lifecycle::path_current_section(bytes(entity) + 236) ||
                    direction != at<Dword>(entity, 240)))
        lifecycle::bind_path(path, reference, sample, direction, entity);
}

void __fastcall brake_path_entity(void *entity, void *, Dword *flags) {
    pending::path_frame::brake_entity(entity, flags);
}
} // namespace nfs::game::runtime::ai::entities
