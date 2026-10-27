#include "nfs/physics/frame/simulation_runtime.hpp"
#include <mmsystem.h>
namespace nfs::game::runtime::race_audio::pending {
// The saved CD-audio mixer enumeration, including its allocation and failure
// paths.
void set_cd_audio_volume(Dword level) {
    Dword devices = mixerGetNumDevs();
    if (!devices)
        return;
    HGLOBAL scratch = GlobalAlloc(GMEM_ZEROINIT, devices * 48);
    if (!scratch)
        return;
    for (Dword index = 0; index < devices; ++index) {
        auto mixer = reinterpret_cast<HMIXEROBJ>(index);
        MIXERLINEA line;
        line.cbStruct = 168;
        line.dwComponentType = 4101;
        if (mixerGetLineInfoA(mixer, &line, 3))
            continue;
        MIXERCONTROLA control;
        MIXERLINECONTROLSA controls;
        controls.cbStruct = 24;
        controls.dwLineID = line.dwLineID;
        controls.dwControlType = 1342373889;
        controls.cControls = 1;
        controls.cbmxctrl = 148;
        controls.pamxctrl = &control;
        if (mixerGetLineControlsA(mixer, &controls, 2))
            continue;
        MIXERCONTROLDETAILS details;
        details.cbStruct = 24;
        details.dwControlID = control.dwControlID;
        details.cChannels = 1;
        details.cbDetails = 4;
        details.cMultipleItems =
            control.cMultipleItems & (Dword(0) - Dword((control.cMultipleItems & 2) != 0));
        SIZE_T bytes = control.fdwControl & 2 ? details.cMultipleItems * 4 : 4;
        details.paDetails = GlobalAlloc(GMEM_ZEROINIT, bytes);
        if (!mixerGetControlDetailsA(mixer, &details, 0)) {
            *static_cast<int *>(details.paDetails) = int(level * 65535) / 127;
            mixerSetControlDetails(mixer, &details, 0);
        }
        if (details.paDetails)
            GlobalFree(details.paDetails);
    }
    GlobalFree(scratch);
}
} // namespace nfs::game::runtime::race_audio::pending
