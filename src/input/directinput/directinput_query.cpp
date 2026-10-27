#include "nfs/input/directinput/directinput.hpp"
namespace nfs::input {
DeviceAbout device_about{};
// GetProperty writes the fifth word. It remains uninitialized if the API fails without writing
// it.
Dword device_axis_mode(IDirectInputDevice2A *device) {
    DIPROPDWORD property;
    property.diph.dwSize = 20;
    property.diph.dwHeaderSize = 16;
    property.diph.dwObj = 0;
    property.diph.dwHow = 0;
    if (device->GetProperty(*reinterpret_cast<const GUID *>(2), &property.diph) == 0) {
        if (property.dwData == 0)
            return 2;
        if (property.dwData == 1)
            return 3;
    }
    return 0;
}

void *device_get(int index, Dword property) {
    if (device_count <= index)
        return nullptr;
    DeviceSlot *slot = devices + index;
    if (!slot->device)
        return nullptr;
    switch (property) {
    case 1:
        return reinterpret_cast<void *>(device_count);
    case 2:
        return describe_device(slot->device, &device_about) == 0 ? &device_about : nullptr;
    case 6: {
        void *buffer = nullptr;
        Dword bytes = 0;
        switch (slot->type & 255) {
        case 1:
        case 4:
            bytes = 80;
            buffer = slot->data;
            break;
        case 2:
            bytes = 16;
            buffer = slot->data;
            break;
        case 3:
            bytes = 256;
            buffer = slot->data;
            break;
        }
        poll_state(slot->device, buffer, bytes);
        return &slot->type;
    }
    case 8:
        return reinterpret_cast<void *>(device_axis_mode(slot->device));
    default:
        kernel::error("[INPUT] - Invalid getstate (%d)\n", property);
        return nullptr;
    }
}
} // namespace nfs::input
