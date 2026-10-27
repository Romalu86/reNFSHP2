#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include <cmath>
namespace nfs::game::runtime::pending {
void reset_car_entry(Dword *);
void clear_car_skids_entry(Dword *);
void update_car_reset_entry(Dword *);
void update_car_effects_entry(Dword *);

void prepare_cars(Dword *manager) {
    for (int index = 0, count = at<int>(manager, 4); index < count; ++index) {
        Dword *car = at<Dword *>(manager, 20 + index * 4);
        Dword *state = at<Dword *>(car, 1768);
        if (at<Byte>(state, 728)) {
            const float axis[] = {0, 0, 1};
            dot_product_entry(reinterpret_cast<float *>(bytes(state) + 716), axis);
        }
    }
}

void finish_cars(Dword *manager) {
    const int count = at<int>(manager, 4);
    for (int index = 0; index < count; ++index) {
        Dword *car = at<Dword *>(manager, 20 + index * 4);
        at<Dword>(car, 3208) = 0;
        if (at<Dword>(car, 1800))
            --at<Dword>(car, 1800);
        Dword *state = at<Dword *>(car, 1768);
        if (!at<Byte>(state, 728)) {
            if (at<int>(car, 1796) > 0) {
                at<Dword>(car, 1796) = 0;
                reset_car_entry(car);
            }
            clear_car_skids_entry(car);
            at<Dword>(car, 1800) = 0;
            kernel::memory_fill(bytes(at<Dword *>(car, 1768)) + 604, 0, 72);
        } else {
            const float value = at<float>(state, 608);
            if (value != 0)
                if (void *interface = at<void *>(car, 7988))
                    invoke(interface, 37, bytes(state) + 604);
            update_car_reset_entry(car);
            physics::update_car_damage(reinterpret_cast<Dword *>(bytes(car) + 7952));
            update_car_effects_entry(car);
        }
    }
}
} // namespace nfs::game::runtime::pending
