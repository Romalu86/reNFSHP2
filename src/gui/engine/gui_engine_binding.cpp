#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::gui {

void __fastcall reset_engine_queue_counts(Object *engine) {
    game::runtime::at<Dword>(engine, 644) = 0;
    game::runtime::at<Dword>(engine, 1288) = 0;
}
// Set the shared owner to a non-null object.
void __fastcall bind_engine(Object *engine) {
    engine_object = engine;
}

void __fastcall unbind_engine(Object *engine) {
    if (engine_object == engine)
        engine_object = nullptr;
}

Object *active_engine() {
    return engine_object;
}
} // namespace nfs::gui
