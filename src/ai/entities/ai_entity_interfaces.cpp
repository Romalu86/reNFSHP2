#include "ai_entity_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::entities {
// EAX=0 leaves are the base interface queries.
Dword __fastcall base_player_view(void *, void *) {
    return 0;
}

Dword __fastcall base_racer_view(void *, void *) {
    return 0;
}

Dword __fastcall base_cop_view(void *, void *) {
    return 0;
}

Dword __fastcall base_perpetrator_view(void *, void *) {
    return 0;
}

Dword __fastcall base_net_view(void *, void *) {
    return 0;
}

Dword __fastcall base_traffic_view(void *, void *) {
    return 0;
}
// Complete RET4 leaf.
void __fastcall base_collision_event(void *, void *, Dword) {
}
// Result is AL.
Byte __fastcall base_accept_event(void *, void *, Dword) {
    return 0;
}

Byte __fastcall base_is_active(void *, void *) {
    return 0;
}

Dword __fastcall body_position(void *view, void *) {
    void *body = invoke<void *>(view, 0);
    float *position = reinterpret_cast<float *>(bytes(view) + 4);
    if (body) {
        const Dword x = at<Dword>(body, 680);
        const float z = at<float>(body, 688);
        at<Dword>(position, 0) = x;
        const float reflected_z = -z;
        const Dword y = at<Dword>(body, 684);
        position[2] = reflected_z;
        at<Dword>(position, 4) = y;
    } else
        position[0] = position[1] = position[2] = 0;
    return reinterpret_cast<Dword>(position);
}

Dword __fastcall body_heading(void *view, void *, float *output) {
    void *body = invoke<void *>(view, 0);
    if (body) {
        const Dword x = at<Dword>(body, 716);
        at<Dword>(output, 0) = x;
        const Dword y = at<Dword>(body, 720);
        const Dword z = at<Dword>(body, 724);
        at<Dword>(output, 4) = y;
        at<Dword>(output, 8) = z;
        output[2] = -output[2];
    } else
        output[0] = output[1] = output[2] = 0;
    return reinterpret_cast<Dword>(output);
}

Dword path_owner(void *view) {
    return reinterpret_cast<Dword>(bytes(view) - 328);
}

Dword __fastcall path_owner_view(void *view, void *) {
    return path_owner(adjust_interface(view));
}
// Complete RET4 leaf.
void __fastcall base_direction_event(void *, void *, Dword) {
}

Dword __fastcall entity_position(void *entity, void *) {
    return body_position(interface_at(entity, 3), nullptr);
}

Dword __fastcall path_position_entry(void *view, void *) {
    return entity_position(bytes(adjust_interface(view)) - 336, nullptr);
}

long double __fastcall entity_forward_speed(void *entity, void *) {
    return at<float>(invoke<void *>(entity, 20), 8);
}

void __fastcall set_entity_velocity(void *entity, void *, const Dword *velocity) {
    void *destination = invoke<void *>(entity, 21);
    at<Dword>(destination, 0) = velocity[0];
    at<Dword>(destination, 4) = velocity[1];
    at<Dword>(destination, 8) = velocity[2];
}

Dword __fastcall entity_uses_path(void *, void *) {
    return 1;
}

Dword __fastcall entity_velocity(void *entity, void *) {
    return reinterpret_cast<Dword>(bytes(entity) + 308);
}

void __fastcall set_path_velocity(void *entity, void *, const Dword *velocity) {
    at<Dword>(entity, 308) = velocity[0];
    const Dword y = velocity[1], z = velocity[2];
    at<Dword>(entity, 312) = y;
    at<Dword>(entity, 316) = z;
}

Dword __fastcall delete_path_entity(void *entity, void *, Dword flags) {
    destroy_path_entity(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}

void __fastcall reset_entity(void *entity, void *) {
    at<Dword>(entity, 8) = 0;
}

void __fastcall set_entity_active(void *entity, void *, Dword active) {
    at<Byte>(entity, 212) = Byte(active);
    if (!Byte(active))
        at<Dword>(entity, 8) = 0;
}

Dword __fastcall racer_entity_view(void *entity, void *) {
    return invoke<Dword>(interface_at(entity, 1), 0) == 0 ? reinterpret_cast<Dword>(entity) : 0;
}

Dword __fastcall player_entity_view(void *entity, void *) {
    return invoke<Dword>(interface_at(entity, 1), 0) == 0 ? reinterpret_cast<Dword>(entity) : 0;
}

Dword __fastcall cop_entity_view(void *entity, void *) {
    return invoke<Dword>(interface_at(entity, 1), 0) == 1 ? reinterpret_cast<Dword>(entity) : 0;
}

Dword __fastcall perpetrator_entity_view(void *entity, void *) {
    return invoke<Dword>(interface_at(entity, 1), 0) == 1 ? reinterpret_cast<Dword>(entity) : 0;
}

Dword __fastcall base_route_section(void *, void *) {
    return 0;
}

Dword __fastcall base_route_sample(void *, void *) {
    return Dword(-1);
}

Dword __fastcall delete_entity(void *entity, void *, Dword flags) {
    destroy_entity(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}

void __fastcall set_helicopter_velocity(void *entity, void *, Dword x, Dword y, Dword z) {
    at<Dword>(entity, 356) = x;
    at<Dword>(entity, 360) = y;
    at<Dword>(entity, 364) = z;
}

Dword __fastcall helicopter_rotation(void *entity, void *) {
    return reinterpret_cast<Dword>(bytes(invoke<void *>(interface_at(entity, 1, 320), 0)) + 72);
}

Dword __fastcall helicopter_body_rotation(void *entity, void *) {
    return reinterpret_cast<Dword>(bytes(invoke<void *>(interface_at(entity, 1, 320), 1)) + 72);
}

Dword flight_body(void *view) {
    return *reinterpret_cast<Dword *>(bytes(view) - 8);
}

Dword flight_mutable_body(void *view) {
    return *reinterpret_cast<Dword *>(bytes(view) - 8);
}

Dword __fastcall flight_body_view(void *view, void *) {
    return flight_body(adjust_interface(view));
}

Dword __fastcall flight_mutable_body_view(void *view, void *) {
    return flight_mutable_body(adjust_interface(view));
}

Dword __fastcall flight_position(void *view, void *) {
    return entity_position(bytes(view) - 332, nullptr);
}

Dword __fastcall flight_owner_view(void *view, void *) {
    return path_owner(bytes(adjust_interface(view)) - 28);
}

Dword __fastcall helicopter_body_entry(void *view, void *) {
    return flight_body(bytes(adjust_interface(view)) - 1352);
}

Dword __fastcall helicopter_mutable_body_entry(void *view, void *) {
    return flight_mutable_body(bytes(adjust_interface(view)) - 1352);
}
void helicopter_heading(void *, float *);

Dword __fastcall helicopter_heading_entry(void *view, void *, float *heading) {
    helicopter_heading(adjust_interface(view), heading);
    return reinterpret_cast<Dword>(heading);
}

Dword __fastcall helicopter_position_entry(void *view, void *) {
    return entity_position(bytes(view) - 1684, nullptr);
}

Dword helicopter_kind(void *) {
    return 7;
}

Dword __fastcall helicopter_kind_entry(void *view, void *) {
    return helicopter_kind(adjust_interface(view));
}

Dword __fastcall helicopter_owner_entry(void *view, void *) {
    return path_owner(bytes(adjust_interface(view)) - 1384);
}

Dword __fastcall delete_helicopter(void *entity, void *, Dword flags) {
    destroy_helicopter(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}

Dword __fastcall delete_flight_entity(void *entity, void *, Dword flags) {
    destroy_flight_entity(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}
Dword helicopter_contact_flags = 1;

Dword __fastcall helicopter_contact_mask(void *, void *) {
    return helicopter_contact_flags;
}

Dword __fastcall helicopter_entity_view(void *entity, void *) {
    return reinterpret_cast<Dword>(entity);
}

Dword __fastcall helicopter_flight_state(void *entity, void *) {
    return at<Dword>(entity, 332);
}

Byte __fastcall helicopter_accelerator(void *entity, void *) {
    return at<Byte>(entity, 421);
}

Byte __fastcall helicopter_brake(void *entity, void *) {
    return at<Byte>(entity, 422);
}

long double __fastcall helicopter_steering(void *entity, void *) {
    return at<std::int8_t>(entity, 420);
}

Dword __fastcall helicopter_velocity(void *entity, void *) {
    return reinterpret_cast<Dword>(bytes(entity) + 356);
}

Dword __fastcall helicopter_mutable_velocity(void *entity, void *) {
    return reinterpret_cast<Dword>(bytes(entity) + 356);
}

long double __fastcall helicopter_speed(void *entity, void *) {
    return at<float>(entity, 364);
}
// Complete RET leaf.
void __fastcall advance_projectile(void *, void *) {
}

Dword __fastcall shape_body_view(void *view, void *) {
    return flight_body(bytes(adjust_interface(view)) - 8);
}

Dword __fastcall shape_mutable_body_view(void *view, void *) {
    return flight_mutable_body(bytes(adjust_interface(view)) - 8);
}

Dword __fastcall shape_position_entry(void *view, void *) {
    return shape_position(bytes(adjust_interface(view)) - 240, nullptr);
}

Dword projectile_kind(void *) {
    return 9;
}

Dword __fastcall projectile_kind_entry(void *view, void *) {
    return projectile_kind(adjust_interface(view));
}

Dword __fastcall projectile_body_entry(void *view, void *) {
    return flight_body(bytes(adjust_interface(view)) - 48);
}

Dword __fastcall projectile_mutable_body_entry(void *view, void *) {
    return flight_mutable_body(bytes(adjust_interface(view)) - 48);
}

Dword __fastcall projectile_position_entry(void *view, void *) {
    return shape_position(bytes(adjust_interface(view)) - 280, nullptr);
}

Dword __fastcall shape_position(void *entity, void *) {
    return body_position(interface_at(entity, 3), nullptr);
}

Dword __fastcall delete_projectile(void *entity, void *, Dword flags) {
    destroy_projectile(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}

Dword __fastcall delete_shape_entity(void *entity, void *, Dword flags) {
    destroy_shape_entity(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}

Dword __fastcall delete_spikes(void *entity, void *, Dword flags) {
    destroy_spikes(entity);
    if (flags & 1)
        gui::application_release(entity);
    return reinterpret_cast<Dword>(entity);
}

Dword spike_kind(void *) {
    return 8;
}

Dword __fastcall spike_kind_entry(void *view, void *) {
    return spike_kind(adjust_interface(view));
}

Dword __fastcall spike_body_entry(void *view, void *) {
    return flight_body(bytes(adjust_interface(view)) - 72);
}

Dword __fastcall spike_mutable_body_entry(void *view, void *) {
    return flight_mutable_body(bytes(adjust_interface(view)) - 72);
}

Dword __fastcall spike_position_entry(void *view, void *) {
    return shape_position(bytes(adjust_interface(view)) - 304, nullptr);
}

Dword __fastcall spike_route_section(void *entity, void *) {
    void *location = at<void *>(entity, 284);
    return location ? at<Dword>(location, 52) : 0;
}

Dword __fastcall spike_route_sample(void *entity, void *) {
    void *location = at<void *>(entity, 284);
    return location ? at<Dword>(location, 56) : Dword(-1);
}

Dword __fastcall path_current_section_entry(void *entity, void *) {
    return reinterpret_cast<Dword>(lifecycle::path_current_section(bytes(entity) + 236));
}

Dword __fastcall path_current_sample_entry(void *entity, void *) {
    return at<Dword>(entity, 236);
}
} // namespace nfs::game::runtime::ai::entities
