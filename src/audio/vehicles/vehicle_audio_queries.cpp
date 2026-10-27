#include "nfs/audio/speech/speech.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::game::audio::vehicle {

Byte car_audio_suppressed(void *car) {
    void *object = at<void *>(car, 7988);
    if (!object)
        return 0;
    return runtime::invoke<Dword>(object, 11) || runtime::ai::racer(object) ? 1 : 0;
}


Dword pending::create_speech_event(Dword source, Dword target, Dword event) {
    return nfs::speech::create_event_code(source,target,event);
}
} // namespace nfs::game::audio::vehicle
