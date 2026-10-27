#include "vehicle_attachment_transforms.hpp"
namespace nfs::game::world::vehicle_render {
// Compact 3x3 rotation and translation from a padded matrix.
void read_model_transform(const Dword *model, Dword *rotation, Dword *position) {
    rotation[0] = model[0];
    rotation[1] = model[1];
    rotation[2] = model[2];
    rotation[3] = model[4];
    rotation[4] = model[5];
    rotation[5] = model[6];
    rotation[6] = model[8];
    rotation[7] = model[9];
    rotation[8] = model[10];
    position[0] = model[12];
    position[1] = model[13];
    position[2] = model[14];
}

Byte attachment_enabled(void *attachments, Dword index) {
    return at<Byte>(attachments, index);
}

void read_attachment_transform(void *attachments, Dword index, Dword *rotation, Dword *position) {
    auto *model = at<Dword *>(attachments, 48 + index * 8);
    read_model_transform(model + 4, rotation, position);
}

Byte read_vehicle_attachment(void *vehicle, Dword index, Dword *rotation, Dword *position) {
    void *attachments = offset(vehicle, 3480);
    Byte available = attachment_enabled(attachments, index);
    if (!available)
        return available;
    read_attachment_transform(attachments, index, rotation, position);
    return 1;
}
} // namespace nfs::game::world::vehicle_render
