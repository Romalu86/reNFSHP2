#pragma once
#include "frontend_application_widgets.hpp"
#include "frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_hooks.hpp"
#include "../../math/precision/extended_float.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::application::frontend {
struct RaceLoadingScreen {
    GeneralWidget base;
    Widget *progress;
    int full_width;
    float completed;
};
static_assert(sizeof(RaceLoadingScreen) == 276 &&
              offsetof(RaceLoadingScreen, progress) == 264);
extern const std::array<Method, 47> race_loading_methods;
void *create_race_loading_screen(void *);
} // namespace nfs::application::frontend
