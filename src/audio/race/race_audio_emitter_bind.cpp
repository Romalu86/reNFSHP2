#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::race_audio {
// Character classification uses the CRT locale.
int find_emitter_sound(const char *name) {
    for (int index = 0; index < pending::sound_definition_count; ++index)
        if (!_stricmp(name, reinterpret_cast<const char *>(pending::sound_definitions +
                                                           index * 72)))
            return index;
    return -1;
}

void position_emitter_sound(Dword definition, Dword serial, const float *position,
                            const float *velocity) {
    if (definition == Dword(-1))
        return;
    Dword record[12];
    for (unsigned i = 0; i < 3; ++i) {
        record[i] = at<Dword>(position, i * 4);
        record[3 + i] = at<Dword>(velocity, i * 4);
    }
    record[6] = serial;
    const Byte *source = pending::sound_definitions + definition * 72;
    record[7] = at<Dword>(source, 40);
    record[8] = at<Dword>(source, 44);
    record[9] = at<Dword>(source, 52);
    record[10] = at<Dword>(source, 56);
    record[11] = at<Dword>(source, 60);
    pending::update_stream_emitter(record);
}
} // namespace nfs::game::runtime::race_audio
