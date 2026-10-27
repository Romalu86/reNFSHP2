#include "../entities/ai_entity_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities {
namespace pending {
void emit_body_effect(void *, Dword, float, const float *);
void emit_world_effect(const float *, const float *, Dword, float, const float *, float,
                       float);
void play_projectile_impact(float, float, float, Dword, int);
void notify_projectile_target(void *, Dword);
void apply_projectile_pursuit_damage(void *, float);
} // namespace pending
namespace {
void deactivate_projectile(void *entity) {
    at<Byte>(entity, 232) = 0;
    invoke(entity, 6, 0u);
    at<Dword>(entity, 240) = 0;
    if (void *shape = at<void *>(entity, 236))
        if (void *body = at<void *>(shape, 80)) {
            at<Dword>(body, 676) = 0;
            at<Byte>(body, 728) = 0;
            at<Byte>(body, 729) = 0;
        }
    at<Dword>(entity, 244) = at<Dword>(entity, 248) = at<Dword>(entity, 256) =
        at<Dword>(entity, 252) = 0;
    at<Byte>(entity, 260) = 0;
}
} // namespace

void update_projectile_effects(void *entity) {
    void *body = invoke<void *>(interface_at(entity, 3), 1);
    if (!body)
        return;
    if (at<Dword>(entity, 256)) {
        --at<Dword>(entity, 256);
        pending::emit_body_effect(body, 17, 1.f, saved::zero_vector);
    }
    float direction[] = {0.f, 0.f, 0.f};
    if (at<Dword>(entity, 248)) {
        --at<Dword>(entity, 248);
        direction[1] = 1.f;
        const float *position = reinterpret_cast<float *>(bytes(body) + 680);
        pending::emit_world_effect(position, direction, 16, 1.f, saved::zero_vector, 0, 0);
        pending::emit_world_effect(position, direction, 18, 1.f, saved::zero_vector, 0, 0);
        return;
    }
    direction[1] = at<float>(body, 124) * 0.75f;
    pending::emit_body_effect(body, 15, 1.f, direction);
}

void begin_projectile_detonation(void *entity) {
    if (!at<Dword>(entity, 248)) {
        void *body_interface = interface_at(entity, 3);
        at<Dword>(entity, 248) = 12;
        if (void *body = invoke<void *>(body_interface, 1)) {
            pending::play_projectile_impact(at<float>(body, 680), at<float>(body, 684),
                                            at<float>(body, 688), 49, 100);
            at<Byte>(body, 729) = 0;
        }
    }
    at<Byte>(entity, 260) = 0;
}

void detonate_projectile(void *entity, void *target) {
    if (at<Dword>(entity, 252))
        return;
    if (!at<Dword>(entity, 248)) {
        at<Dword>(entity, 248) = 12;
        if (void *body = invoke<void *>(interface_at(entity, 3), 1)) {
            pending::play_projectile_impact(at<float>(body, 680), at<float>(body, 684),
                                            at<float>(body, 688), 49, 100);
            at<Byte>(body, 729) = 0;
        }
    }
    at<Byte>(entity, 260) = 0;
    at<Dword>(entity, 252) = at<Dword>(entity, 248);
    if (target) {
        if (void *car = at<void *>(target, 368)) {
            void *body = at<void *>(car, 1768);
            const float spin_x = (ai::random_word() & 1) ? -1.f : 1.f;
            at<float>(body, 20) = spin_x + at<float>(body, 20);
            const float spin_z = (ai::random_word() & 1) ? -1.f : 1.f;
            at<float>(body, 28) = spin_z + at<float>(body, 28);
            at<float>(body, 12) = at<float>(body, 12) + 2.f;
        }
        if (void *cop = invoke<void *>(target, 11))
            pending::notify_projectile_target(cop, 4);
    }
}

void *projectile_contact_entity(void *body) {
    ++lifecycle::state<Dword>(7352868);
    void *first = nullptr, *second = nullptr;
    const float contact = at<float>(body, 608);
    if (contact != 0 && at<Dword>(body, 604)) {
        if (void *car = physics::vehicle_car(at<void *>(body, 604)))
            first = at<void *>(car, 7988);
    }
    if (void *car = at<void *>(body, 676)) {
        second = at<void *>(car, 7988);
        at<Dword>(body, 676) = 0;
    }
    return first || !second ? first : second;
}
// The RET4 retains an ignored stack word.
void __fastcall collide_projectile(void *entity, void *, void *) {
    void *shape = at<void *>(entity, 236), *body = shape ? at<void *>(shape, 80) : nullptr;
    if (!body) {
        deactivate_projectile(entity);
        return;
    }
    at<void *>(entity, 224) = body;
    invoke(entity, 6, 1u);
    update_projectile_effects(entity);
    if (!at<Dword>(entity, 252)) {
        const Dword fired_at = at<Dword>(entity, 240);
        const int elapsed =
            fired_at ? int(graphics::unresolved::camera_state - fired_at) : 0;
        if (elapsed > lifecycle::state<int>(7352864)) {
            detonate_projectile(entity, nullptr);
            return;
        }
        if (void *target = projectile_contact_entity(body)) {
            detonate_projectile(entity, target);
            if (void *cop = ai::police(target)) {
                void *owner = at<void *>(entity, 244);
                void *pursued = owner ? at<void *>(owner, 408) : nullptr;
                if (pursued && at<void *>(cop, at<const int *>(cop, 4)[4] + 12) == pursued)
                    pending::apply_projectile_pursuit_damage(cop, at<float>(system(76), 84608));
            }
        }
    } else {
        --at<Dword>(entity, 252);
        if (!at<Dword>(entity, 252))
            deactivate_projectile(entity);
    }
    dependencies::synchronize_track_vehicle(shape);
    physics::mark_track_vehicle_moved(shape);
}
} // namespace nfs::game::runtime::ai::entities
