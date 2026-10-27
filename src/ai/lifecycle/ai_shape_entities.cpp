#include "../entities/ai_entity_runtime.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
namespace nfs::game::runtime::ai::entities {

void __fastcall reset_projectile(void *entity, void *) {
    void *shape = at<void *>(entity, 236), *body = at<void *>(shape, 80);
    if (!body) {
        lifecycle::pending::bind_shape_body(shape, 1);
        body = at<void *>(shape, 80);
        at<Dword>(body, 676) = 0;
    }
    at<Byte>(body, 728) = 0;
    at<Byte>(body, 729) = 0;
    at<void *>(entity, 224) = body;
    at<Byte>(entity, 260) = 0;
    invoke(entity, 6, 0u);
    for (unsigned offset : {244u, 240u, 248u, 256u, 252u})
        at<Dword>(entity, offset) = 0;
}

void __fastcall set_projectile_active(void *entity, void *, Dword active) {
    set_shape_entity_active(entity, nullptr, active);
    if (void *shape = at<void *>(entity, 236)) {
        void *body = invoke<void *>(interface_at(entity, 3), 1);
        if (!body) {
            lifecycle::pending::bind_shape_body(shape, 1);
            at<void *>(entity, 224) = at<void *>(shape, 80);
            body = invoke<void *>(interface_at(entity, 3), 1);
            at<Dword>(body, 676) = 0;
        }
        at<Byte>(body, 728) = Byte(active);
        at<Byte>(body, 729) = at<Byte>(entity, 260);
    }
}
Byte settle_spike_body(void *entity);

void __fastcall advance_spikes(void *entity, void *) {
    if (at<Byte>(entity, 212) && at<Byte>(entity, 233))
        settle_spike_body(entity);
}

void unlink_shape(void *shape) {
    void *next = at<void *>(shape, 12);
    if (next)
        at<Dword>(next, 8) = at<Dword>(shape, 8);
    at<Dword>(shape, 12) = 0;
    if (void *previous = at<void *>(shape, 8)) {
        at<void *>(previous, 12) = next;
        at<Dword>(shape, 8) = 0;
    }
}

void insert_shape_sentinel(void *shape) {
    void *sentinel = physics::pending::global_collision_cell + 8;
    void *head = at<void *>(sentinel, 8);
    at<void *>(shape, 8) = head;
    at<void *>(shape, 12) = sentinel;
    at<void *>(sentinel, 8) = shape;
    if (head)
        at<void *>(head, 12) = shape;
}

void __fastcall reset_helicopter(void *entity, void *) {
    for (unsigned field : {368u, 372u, 376u}) {
        void *shape = at<void *>(entity, field), *body = nullptr;
        if (shape) {
            if (!at<void *>(shape, 80))
                lifecycle::pending::bind_shape_body(shape, 1);
            body = at<void *>(shape, 80);
            at<Byte>(body, 728) = 0;
            at<Byte>(body, 729) = 0;
        }
        if (field == 368)
            at<void *>(entity, 324) = body;
        if (field == 368 || shape) {
            unlink_shape(shape);
            insert_shape_sentinel(shape);
        }
    }
    invoke(entity, 6, 0u);
    const Dword zero_x = at<const volatile Dword>(saved::zero_vector, 0);
    const Dword zero_z = at<const volatile Dword>(saved::zero_vector, 8);
    const Dword zero_y = at<const volatile Dword>(saved::zero_vector, 4);
    auto store_word = [entity](unsigned offset, Dword value) {
        at<volatile Dword>(entity, offset) = value;
    };
    store_word(356, zero_x);
    store_word(360, zero_y);
    store_word(364, zero_z);
    store_word(384, zero_x);
    store_word(388, zero_y);
    store_word(392, zero_z);
    store_word(424, zero_x);
    store_word(428, zero_y);
    store_word(380, 0);
    store_word(396, 0);
    store_word(412, 0);
    store_word(432, zero_z);
    store_word(436, 0);
    store_word(440, 0);
    store_word(444, Dword(-1));
    at<volatile Byte>(entity, 349) = 0;
    store_word(1652, 0);
    store_word(1656, 0);
    at<volatile Byte>(entity, 1660) = 0;
    store_word(1664, 0);
    store_word(352, 0);
    store_word(344, 0);
    store_word(408, 0);
    store_word(332, 1);
    reset_path_entity(entity, nullptr);
    store_word(1668, 0);
    store_word(1676, 0);
    store_word(1672, 3212836864u);
}

void __fastcall set_helicopter_active(void *entity, void *, Dword active) {
    set_path_entity_active(entity, nullptr, active);
    void *body = invoke<void *>(interface_at(entity, 1, 320), 1);
    at<Byte>(body, 728) = 0;
    at<Byte>(body, 729) = Byte(active);
    for (unsigned field : {372u, 376u})
        if (void *shape = at<void *>(entity, field)) {
            body = at<void *>(shape, 80);
            at<Byte>(body, 728) = 0;
            at<Byte>(body, 729) = Byte(active);
        }
}
// Receiver is the flight body's adjusted interface.
void helicopter_heading(void *view, float *heading) {
    const float angle = *reinterpret_cast<float *>(bytes(view) - 1296);
    heading[0] = float(turn_sine(angle));
    heading[1] = 0;
    heading[2] = float(-turn_cosine(angle));
}
} // namespace nfs::game::runtime::ai::entities
