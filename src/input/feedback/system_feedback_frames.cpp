#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include <initializer_list>
namespace nfs::lifecycle::table_entries {
using namespace game::runtime;

void __fastcall reset_feedback_effects(System *object, void *) {
    if (at<int>(object, 4) < 0)
        return;
    for (unsigned offset : {104u, 200u, 8u})
        if (at<int>(object, offset) >= 0)
            input::stop_effect(at<int>(object, offset));
}

void __fastcall unload_feedback_effects(System *object, void *) {
    if (at<int>(object, 4) < 0)
        return;
    for (unsigned offset : {104u, 200u, 8u})
        if (at<int>(object, offset) >= 0)
            input::stop_effect(at<int>(object, offset));
    if (at<int>(object, 4) >= 0) {
        reset_feedback_effects(object, nullptr);
        input::disable_feedback(at<int>(object, 4));
        for (unsigned offset : {4u, 8u, 104u, 200u})
            at<Dword>(object, offset) = 4294967295u;
    }
}
} // namespace nfs::lifecycle::table_entries
