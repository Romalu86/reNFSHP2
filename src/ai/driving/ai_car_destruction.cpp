#include "ai_car_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

void destroy_car_entity(void *entity) {
    bind_tables(entity, car_methods, car_kind_methods, car_casts_methods, car_body_methods);
    set_displacement(entity, 2, 568);
    set_displacement(entity, 3, 576);
    entities::destroy_path_entity(entity);
}

void destroy_cop_base(void *entity) {
    destroy_car_entity(entity);
}

void destroy_racer(void *entity) {
    destroy_car_entity(entity);
}

void destroy_pursuit_car(void *entity) {
    bind_tables(entity, pursuit_methods, pursuit_kind_methods, pursuit_casts_methods,
                pursuit_body_methods, pursuit_perpetrator_methods);
    set_displacement(entity, 1, 640);
    set_displacement(entity, 2, 648);
    set_displacement(entity, 3, 656);
    set_displacement(entity, 4, 676);
    destroy_car_entity(entity);
}

void destroy_network_car(void *entity) {
    bind_tables(entity, network_car_methods, network_car_kind_methods, network_car_casts_methods,
                network_car_body_methods, network_car_perpetrator_methods);
    set_displacement(entity, 1, 592);
    set_displacement(entity, 2, 600);
    set_displacement(entity, 3, 608);
    set_displacement(entity, 4, 628);
    destroy_car_entity(entity);
}

void destroy_traffic_car(void *entity) {
    bind_tables(entity, traffic_methods, traffic_kind_methods, traffic_casts_methods,
                traffic_body_methods);
    set_displacement(entity, 1, 588);
    set_displacement(entity, 2, 596);
    set_displacement(entity, 3, 604);
    destroy_car_entity(entity);
}

void destroy_player(void *entity) {
    bind_tables(entity, player_methods, player_kind_methods, player_casts_methods,
                player_body_methods, player_perpetrator_methods);
    set_displacement(entity, 1, 644);
    set_displacement(entity, 2, 652);
    set_displacement(entity, 3, 660);
    set_displacement(entity, 4, 680);
    destroy_pursuit_car(entity);
}

void destroy_ai_cop(void *entity) {
    bind_tables(entity, ai_cop_methods, ai_cop_kind_methods, ai_cop_casts_methods,
                ai_cop_body_methods, ai_cop_perpetrator_methods);
    set_displacement(entity, 1, 668);
    set_displacement(entity, 2, 676);
    set_displacement(entity, 3, 684);
    set_displacement(entity, 4, 704);
    destroy_pursuit_car(entity);
}

void destroy_ai_racer(void *entity) {
    bind_tables(entity, ai_racer_methods, ai_racer_kind_methods, ai_racer_casts_methods,
                ai_racer_body_methods, ai_racer_perpetrator_methods);
    set_displacement(entity, 1, 604);
    set_displacement(entity, 2, 612);
    set_displacement(entity, 3, 620);
    set_displacement(entity, 4, 640);
    destroy_car_entity(entity);
}

void destroy_autonomous_cop(void *entity) {
    bind_tables(entity, autonomous_cop_methods, autonomous_cop_kind_methods,
                autonomous_cop_casts_methods, autonomous_cop_body_methods,
                autonomous_cop_perpetrator_methods);
    set_displacement(entity, 1, 1856);
    set_displacement(entity, 2, 1864);
    set_displacement(entity, 3, 1872);
    set_displacement(entity, 4, 1892);
    destroy_car_entity(entity);
}

Dword __fastcall car_delete(void *entity, void *, Dword flags) {
    destroy_car_entity(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall cop_base_delete(void *entity, void *, Dword flags) {
    destroy_cop_base(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall player_delete(void *entity, void *, Dword flags) {
    destroy_player(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall pursuit_delete(void *entity, void *, Dword flags) {
    destroy_pursuit_car(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall racer_delete(void *entity, void *, Dword flags) {
    destroy_racer(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall ai_cop_delete(void *entity, void *, Dword flags) {
    destroy_ai_cop(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall network_car_delete(void *entity, void *, Dword flags) {
    destroy_network_car(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall ai_racer_delete(void *entity, void *, Dword flags) {
    destroy_ai_racer(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall traffic_delete(void *entity, void *, Dword flags) {
    destroy_traffic_car(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}

Dword __fastcall autonomous_cop_delete(void *entity, void *, Dword flags) {
    destroy_autonomous_cop(entity);
    if (flags & 1)
        gui::application_release(entity);
    return word_pointer(entity);
}
} // namespace nfs::game::runtime::ai::cars
