#include "nfs/graphics/textures/pixel_codec.hpp"
#include "nfs/graphics/textures/pixel_blocks.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::graphics::surfaces::pixels {
namespace {

class ConstructionStorage {
  public:
    explicit ConstructionStorage(Codec *value) : storage(value) {}
    ~ConstructionStorage() noexcept(false) {
        if (storage)
            gui::application_release(storage);
    }
    void release() { storage = nullptr; }
  private:
    Codec *storage;
};
} // namespace
// Unknown formats take the saved null-return branch.
Dword *create(Image *image) {
    Dword format = image->words[1];
    Codec *result = nullptr;
    Dword size = 0;
    for (Dword supported : linear_formats)
        if (supported == format) {
            size = 4204;
            break;
        }
    if (size) {
        auto *storage = static_cast<Codec *>(gui::application_allocate(size));
        if (storage) {
            ConstructionStorage pending(storage);
            result = construct_linear(storage, image, format);
            pending.release();
        }
    } else {
        switch (format) {
        case 827611204:
        case 844388420:
        case 861165636:
        case 877942852:
        case 894720068:
            size = 4292;
            break;
        case 844715353:
        case 1498831189:
            size = 4252;
            break;
        default:
            return nullptr;
        }
        auto *storage = static_cast<Codec *>(gui::application_allocate(size));
        if (storage) {
            ConstructionStorage pending(storage);
            switch (format) {
            case 827611204:
                result = construct_dxt1(storage, image);
                break;
            case 844388420:
                result = construct_dxt2(storage, image);
                break;
            case 861165636:
                result = construct_dxt3(storage, image);
                break;
            case 877942852:
                result = construct_dxt4(storage, image);
                break;
            case 894720068:
                result = construct_dxt5(storage, image);
                break;
            case 844715353:
                result = construct_yuy2(storage, image);
                break;
            case 1498831189:
                result = construct_uyvy(storage, image);
                break;
            }
            pending.release();
        }
    }
    if (result && result->words[4]) {
        using Quantize = void(__thiscall *)(Codec *);
        reinterpret_cast<const Quantize *>(result->words[0])[3](result);
    }
    return reinterpret_cast<Dword *>(result);
}
} // namespace nfs::graphics::surfaces::pixels
