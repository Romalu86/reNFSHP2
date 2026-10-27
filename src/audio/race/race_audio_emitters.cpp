#include "race_audio_distance.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::runtime::race_audio::pending {
namespace {
// The second and third source DWORDs are captured before either is written.
void copy_emitter_vector(void *destination, const void *source) {
    at<volatile Dword>(destination, 0) = at<volatile Dword>(source, 0);
    const Dword second = at<volatile Dword>(source, 4);
    const Dword third = at<volatile Dword>(source, 8);
    at<volatile Dword>(destination, 4) = second;
    at<volatile Dword>(destination, 8) = third;
}
}
// Register positional emitters or immediately submit flagged sounds.
void update_stream_emitter(const void *emitter) {
    for (int listener = 0; listener < race_audio_listener_count; ++listener) {
        int distance;
        const Dword initial_flags = at<Dword>(emitter, 28);
        if (!(initial_flags & 4096) || !(initial_flags & 512)) {
            Dword *camera = invoke<Dword *>(tick_services[listener], 2);
            float position[3], orientation[9];
            for (unsigned axis = 0; axis < 3; ++axis)
                at<Dword>(position, axis * 4) = at<Dword>(camera, 4 + axis * 4);
            camera_orientation(camera, orientation);
            distance = rounded_point_distance(position, static_cast<const float *>(emitter));
            if (distance < 0 || distance > 10000)
                distance = 10000;
        } else {
            void *car = physics::local_car(system(68), listener);
            const float *position =
                reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 680);
            distance = rounded_point_distance(position, static_cast<const float *>(emitter));
        }
        Dword radius = at<Dword>(emitter, 36), squared = Dword(distance) * Dword(distance);
        if (static_cast<int>(squared) > static_cast<int>(radius * radius))
            continue;
        Dword listener_bits = listener ? 0 : 256; // (listener != 0) - 1, then AND 100h.
        Dword flags = at<Dword>(emitter, 28);
        if (flags & 512) {
            Dword pitch =
                Dword(doppler(static_cast<const float *>(emitter),
                             reinterpret_cast<const float *>(bytes(emitter) + 12),
                             reinterpret_cast<const float *>(listener_configuration)));
            Dword pan = spatial_pan(listener_state, emitter);
            flags = at<Dword>(emitter, 28);
            if (flags & 1024)
                pan = 0;
            if (race_audio_listener_count == 2)
                pan = listener ? 16383 : 49152;
            Dword channel;
            if (flags & 4)
                channel = 118;
            else if (flags & 8)
                channel = 117;
            else if (flags & 1)
                channel = 113;
            else if (flags & 2)
                channel = 120;
            else
                return;
            submit_distanced(channel | listener_bits, at<Dword>(emitter, 32), 64, pitch, squared,
                             pan, radius * radius, at<Dword>(emitter, 40), at<Dword>(emitter, 44),
                             (flags >> 11) & 4294967041u);
            return;
        }
        Byte *state = listener_state + Dword(listener) * 2760;
        int count = at<int>(state, 580);
        if (count > 31)
            return;
        int slot = 0;
        for (; slot < count; ++slot)
            if (at<Dword>(state, 608 + slot * 48) == at<Dword>(emitter, 24))
                break;
        const bool existing = slot < count;
        if (!existing)
            slot = count;
        Byte *destination = state + 584 + Dword(slot) * 48;
        copy_emitter_vector(destination, emitter);
        at<volatile Dword>(destination, 24) = at<volatile Dword>(emitter, 24);
        copy_emitter_vector(destination + 12, bytes(emitter) + 12);
        if (existing) {
            const Dword current_flags = at<volatile Dword>(emitter, 28);
            const Dword event = at<volatile Dword>(emitter, 32);
            at<volatile Dword>(destination, 28) = current_flags;
            const Dword current_radius = at<volatile Dword>(emitter, 36);
            at<volatile Dword>(destination, 32) = event;
            const Dword maximum = at<volatile Dword>(emitter, 40);
            at<volatile Dword>(destination, 36) = current_radius;
            const Dword minimum = at<volatile Dword>(emitter, 44);
            at<volatile Dword>(destination, 40) = maximum;
            at<volatile Dword>(destination, 44) = minimum;
            return;
        }
        at<volatile Dword>(destination, 28) = at<volatile Dword>(emitter, 28);
        at<volatile Dword>(destination, 32) = at<volatile Dword>(emitter, 32);
        at<volatile Dword>(destination, 36) = radius;
        at<volatile Dword>(destination, 40) = at<volatile Dword>(emitter, 40);
        at<volatile Dword>(destination, 44) = at<volatile Dword>(emitter, 44);
        ++at<volatile Dword>(state, 580);
    }
}
} // namespace nfs::game::runtime::race_audio::pending
