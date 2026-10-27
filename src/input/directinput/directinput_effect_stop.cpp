#include "nfs/input/directinput/directinput.hpp"
namespace nfs::input {

void stop_effect(int index) {
    if (index >= 0 && index < 32 && effect_slots()[index].device_index != -1) {
        effect_slots()[index].effect->Stop();

        effect_stop_state(index) = 0;
    }
}
} // namespace nfs::input
