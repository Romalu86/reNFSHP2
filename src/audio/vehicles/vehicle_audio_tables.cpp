#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::game::audio::vehicle {

void bind_engine_tables(int listener) {
    for (int side = 0; side < 2; ++side) {
        Byte *data = table(side ? 7218896 : 7218888, listener);
        table(7218904, listener * 2 + side) = data;
        if (!data[5]) {
            for (unsigned offset = 328; offset < 360; offset += 4) {
                at<Dword>(data, offset - 32) += reinterpret_cast<Dword>(data + offset - 32);
                at<Dword>(data, offset) += reinterpret_cast<Dword>(data + offset);
            }
            data[5] = 1;
        }
    }
    auto *engine = owner(listener);
    at<int>(engine, 4) = 0;
    at<std::int16_t>(engine, 20) = 0;
    at<Byte>(engine, 19) = 0;
    for (int layer = 0; layer < 16; ++layer) {
        // The shipped table scan uses layer / 8 here, not layer % 8.
        if (at<std::int8_t>(table(7218904, listener * 2 + (layer > 7)), 32 + layer / 8) >= 0)
            at<Byte>(engine, 19) = Byte(layer + 1);
        at<int>(engine, 52 + layer * 12) = at<int>(engine, 244 + layer * 12) = -1;
    }
}
} // namespace nfs::game::audio::vehicle
