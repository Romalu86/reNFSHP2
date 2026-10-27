#include "ai_car_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::game::runtime::ai::cars {

void clear_tire_damage(void *damage) {
    at<Byte>(damage, 0) = 0;
    at<Dword>(damage, 4) = Dword(-1);
}

void __fastcall car_reset(void *entity, void *) {
    at<Dword>(entity, 404) = 0;
    entities::reset_path_entity(entity, nullptr);
    at<Dword>(entity, 380) = 0;
    at<Byte>(entity, 408) = 0;
    at<Dword>(entity, 392) = 0;
    at<Dword>(entity, 396) = 0;
    at<Dword>(entity, 400) = 0;
    at<Dword>(entity, 404) = 0;
    at<Dword>(entity, 384) = 0;
    at<Byte>(entity, 409) = 0;
    at<Dword>(entity, 556) = 0;
    at<Dword>(entity, 560) = 0;
    at<Dword>(entity, 420) = Dword(-1);
    at<Dword>(entity, 424) = Dword(-1);
    at<Dword>(entity, 372) = Dword(-1);
    at<Dword>(entity, 376) = Dword(-1);
    at<Dword>(entity, 412) = Dword(-1);
    at<Dword>(entity, 388) = 0;
}

void __fastcall cop_base_reset(void *entity, void *) {
    car_reset(entity, nullptr);
    perpetrator_reset(entities::interface_at(entity, 4), nullptr);
}

void __fastcall reset_racer(void *entity, void *) {
    at<Dword>(entity, 564) = 3212836864u;
    at<Dword>(entity, 568) = Dword(-1);
    at<Dword>(entity, 572) = 0;
    at<Dword>(entity, 584) = 0;
    at<Dword>(entity, 592) = 0;
    at<Byte>(entity, 580) = 0;
    lifecycle::clear_path(bytes(entity) + 220);
    car_reset(entity, nullptr);
    perpetrator_reset(entities::interface_at(entity, 4), nullptr);
    at<Byte>(entity, 596) = 0;
    invoke(entity, 6, Dword{1});
    if (void *car = at<void *>(entity, 368))
        clear_tire_damage(bytes(car) + 7952);
}

void __fastcall reset_pursuit_car(void *entity, void *) {
    at<Dword>(entity, 604) = 0;
    at<Dword>(entity, 608) = 0;
    at<Dword>(entity, 624) = 0;
    at<Dword>(entity, 612) = 0;
    at<Dword>(entity, 616) = 0;
    at<Dword>(entity, 620) = 0;
    at<Dword>(entity, 600) = 1;
    at<Dword>(entity, 628) = 0;
    at<Dword>(entity, 632) = 0;
    at<Dword>(entity, 636) = 0;
    reset_racer(entity, nullptr);
}

void __fastcall reset_autonomous_cop(void *entity, void *) {
    at<Dword>(entity, 568) = 0;
    at<Dword>(entity, 572) = 0;
    at<Dword>(entity, 588) = 0;
    at<Dword>(entity, 580) = 0;
    at<Byte>(entity, 584) = 0;
    at<Dword>(entity, 592) = 0;
    at<Dword>(entity, 596) = 0;
    at<Dword>(entity, 1808) = 1;
    cop_base_reset(entity, nullptr);
    at<Byte>(entity, 1804) = 0;
    at<Dword>(entity, 1812) = 0;
    at<Dword>(entity, 1816) = 0;
    at<Dword>(entity, 1820) = 0;
    at<Dword>(entity, 1824) = 0;
    at<Dword>(entity, 1828) = 0;
    at<Dword>(entity, 1832) = 0;
    at<Dword>(entity, 1836) = 0;
    at<Byte>(entity, 1840) = 1;
    at<Dword>(entity, 1844) = Dword(-1);
    at<Dword>(entity, 1848) = 0;
    at<Dword>(entity, 1852) = 0;
}

void __fastcall ai_cop_reset(void *entity, void *) {
    at<Byte>(entity, 640) = 0;
    reset_pursuit_car(entity, nullptr);
    at<Byte>(entity, 656) = 1;
    at<Dword>(entity, 660) = 0;
    at<Dword>(entity, 664) = 0;
    invoke(entity, 6, Dword{0});
    at<Dword>(entity, 652) = 0;
    at<Dword>(entity, 404) = 4;
}

void __fastcall reset_network_car(void *entity, void *) {
    at<Dword>(entity, 568) = 0;
    at<Byte>(entity, 580) = 0;
    at<Byte>(entity, 581) = 0;
    at<Byte>(entity, 582) = 0;
    at<Byte>(entity, 583) = 0;
    at<Dword>(entity, 584) = 3212836864u;
    cop_base_reset(entity, nullptr);
    at<Byte>(entity, 572) = 0;
    at<Byte>(entity, 573) = 0;
    at<Byte>(entity, 574) = 0;
    at<Dword>(entity, 576) = 0;
}

void __fastcall ai_racer_reset(void *entity, void *) {
    at<Byte>(entity, 600) = 0;
    reset_racer(entity, nullptr);
    application::pending::set_actor_finished(entity, 0);
}

void __fastcall traffic_reset(void *entity, void *) {
    car_reset(entity, nullptr);
    void *body = at<void *>(at<void *>(entity, 368), 1768);
    at<Byte>(body, 728) = 0;
    at<Byte>(body, 729) = 0;
    at<Dword>(entity, 564) = 0;
    at<Dword>(entity, 568) = 0;
    at<Dword>(entity, 584) = 0;
    invoke(entity, 6, Dword{0});
    at<Dword>(entity, 584) = 0;
    at<Dword>(entity, 580) = Dword(-1);
}
} // namespace nfs::game::runtime::ai::cars
