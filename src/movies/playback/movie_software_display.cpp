#include "movie_runtime.hpp"
#include "movie_software_display.hpp"
#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/input/window/window_input.hpp"
#if defined(_MSC_VER)
using WINBOOL = BOOL;
#endif
#include "../../third_party/d3d8/d3d8.h"
#include <climits>
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

namespace nfs::movies {
namespace {
bool needs_recovery(HRESULT result) noexcept {
    return result == D3DERR_DEVICELOST || result == D3DERR_DEVICENOTRESET;
}

void require_success(HRESULT result, const char *message) {
    if (FAILED(result))
        throw std::runtime_error(message);
}

class SurfaceReference {
  public:
    SurfaceReference() noexcept = default;
    SurfaceReference(const SurfaceReference &) = delete;
    SurfaceReference &operator=(const SurfaceReference &) = delete;
    IDirect3DSurface8 *value = nullptr;
    ~SurfaceReference() noexcept { reset(); }
    void reset() noexcept {
        if (value) {
            value->Release();
            value = nullptr;
        }
    }
};

class ImageLock {
  public:
    explicit ImageLock(IDirect3DSurface8 *image) noexcept : image_(image) {}
    ImageLock(const ImageLock &) = delete;
    ImageLock &operator=(const ImageLock &) = delete;
    ~ImageLock() noexcept {
        if (image_)
            image_->UnlockRect();
    }
    HRESULT unlock() noexcept {
        IDirect3DSurface8 *image = image_;
        image_ = nullptr;
        return image->UnlockRect();
    }

  private:
    IDirect3DSurface8 *image_;
};

unsigned pixel_bytes(std::uint32_t format) {
    switch (format) {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
        return 4;
    case D3DFMT_R8G8B8:
        return 3;
    case D3DFMT_R5G6B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A4R4G4B4:
    case D3DFMT_X4R4G4B4:
    case D3DFMT_A8R3G3B2:
        return 2;
    default:
        throw std::runtime_error("MAD unsupported backbuffer pixel format");
    }
}

template <std::uint16_t (*Pack)(std::uint32_t)>
void write_row16(std::uint8_t *destination, const std::uint32_t *first,
                 const std::uint32_t *second, unsigned width, unsigned weight) {
    for (unsigned x = 0; x < width; ++x) {
        const std::uint16_t pixel = Pack(display_pixels::blend(first[x], second[x], weight));
        std::memcpy(destination + std::size_t(x) * 2, &pixel, sizeof(pixel));
    }
}

void write_row(std::uint8_t *destination, const std::uint32_t *first,
               const std::uint32_t *second, unsigned width, unsigned weight,
               std::uint32_t format) {
    // Select the destination representation once per row, outside the pixel loop.
    switch (format) {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
        if (!weight) {
            std::memcpy(destination, first, std::size_t(width) * 4);
        } else {
            for (unsigned x = 0; x < width; ++x) {
                const std::uint32_t pixel = display_pixels::blend(first[x], second[x], weight);
                std::memcpy(destination + std::size_t(x) * 4, &pixel, sizeof(pixel));
            }
        }
        break;
    case D3DFMT_R8G8B8:
        for (unsigned x = 0; x < width; ++x) {
            const std::uint32_t pixel = display_pixels::blend(first[x], second[x], weight);
            destination[std::size_t(x) * 3] = static_cast<std::uint8_t>(pixel);
            destination[std::size_t(x) * 3 + 1] = static_cast<std::uint8_t>(pixel >> 8);
            destination[std::size_t(x) * 3 + 2] = static_cast<std::uint8_t>(pixel >> 16);
        }
        break;
    case D3DFMT_R5G6B5:
        write_row16<display_pixels::pack565>(destination, first, second, width, weight);
        break;
    case D3DFMT_A1R5G5B5:
    case D3DFMT_X1R5G5B5:
        write_row16<display_pixels::pack1555>(destination, first, second, width, weight);
        break;
    case D3DFMT_A4R4G4B4:
    case D3DFMT_X4R4G4B4:
        write_row16<display_pixels::pack4444>(destination, first, second, width, weight);
        break;
    case D3DFMT_A8R3G3B2:
        write_row16<display_pixels::pack332>(destination, first, second, width, weight);
        break;
    default:
        throw std::runtime_error("MAD unsupported backbuffer pixel format");
    }
}
} // namespace

SoftwareMovieDisplay *SoftwareMovieDisplay::active_ = nullptr;

SoftwareMovieDisplay::SoftwareMovieDisplay(void *first_frame) {
    // The dispatcher returns zero without writing when MMX is disabled. Do not treat that no-output
    // branch as a valid decoded display image.
    if (!kernel::cpu_mmx)
        throw std::runtime_error("MAD colour conversion requires the MMX capability");
    auto *shape = field<void *>(first_frame, 4);
    width_ = field<std::int16_t>(shape, 4);
    height_ = field<std::int16_t>(shape, 6);
    if (width_ <= 0 || height_ <= 0)
        throw std::runtime_error("MAD invalid decoded dimensions");
    // The MMX converter requires an eight-byte destination pitch/alignment.
    stride_ = ((width_ + 1) & ~1) * 4;
    const std::uint64_t bytes = std::uint64_t(stride_) * unsigned(height_) + 7;
    if (bytes > INT_MAX)
        throw std::bad_alloc();
    allocation_ = memory.allocate("MAD CPU frame", int(bytes), 0, 0, memory.pool);
    if (!allocation_)
        throw std::bad_alloc();
    pixels_ = reinterpret_cast<Byte *>((reinterpret_cast<std::uintptr_t>(allocation_) + 7) &
                                      ~std::uintptr_t(7));
    previous_active_ = active_;
    previous_redraw_ = window_input::redraw;
    active_ = this;
    window_input::redraw = request_redraw;
    owns_redraw_ = true;
}

void __cdecl SoftwareMovieDisplay::request_redraw() noexcept {
    // Reset and DispatchMessage can deliver paint/move synchronously. Never enter
    // Direct3D from the window callback; the owning movie loop performs the copy.
    if (active_)
        InterlockedExchange(&active_->repaint_, 1);
}

void SoftwareMovieDisplay::release_device() noexcept {
    if (image_) {
        image_->Release();
        image_ = nullptr;
    }
    if (device_) {
        device_->Release();
        device_ = nullptr;
    }
    image_width_ = image_height_ = image_format_ = 0;
    image_dirty_ = true;
}

void SoftwareMovieDisplay::release() noexcept {
    if (owns_redraw_) {
        if (active_ == this) {
            active_ = previous_active_;
            if (window_input::redraw == request_redraw)
                window_input::redraw = previous_redraw_;
        }
        owns_redraw_ = false;
    }
    release_device();
    if (allocation_) {
        memory.release(allocation_);
        allocation_ = nullptr;
        pixels_ = nullptr;
    }
}

SoftwareMovieDisplay::~SoftwareMovieDisplay() noexcept { release(); }

void SoftwareMovieDisplay::convert(void *frame) {
    auto *shape = field<void *>(frame, 4);
    if (field<std::int16_t>(shape, 4) != width_ || field<std::int16_t>(shape, 6) != height_)
        throw std::runtime_error("MAD decoded dimensions changed");
    const auto *luma = static_cast<const Byte *>(shape) +
                       ((field<Dword>(shape, 12) & 4096) ? field<Dword>(shape, 16) : 16);
    const auto *chroma = luma + Dword(width_) * Dword(height_);
    const auto *second_chroma = chroma + Dword(width_ / 2) * Dword(height_ / 2);
    const Dword result = pending::convert_yuv(luma, chroma, second_chroma, Dword(width_),
                                             Dword(width_), height_, pixels_, Dword(stride_), 1);
    if (result)
        throw std::runtime_error("MAD software colour conversion failed");
    row_index_[0] = row_index_[1] = -1;
    image_dirty_ = pending_ = has_frame_ = true;
}

bool SoftwareMovieDisplay::needs_present() const noexcept {
    return has_frame_ && (pending_ || repaint_ != 0);
}

void SoftwareMovieDisplay::prepare_for_recovery() noexcept {
    // No backbuffer reference or locked surface survives this operation.
    // The copied CPU frame remains owned and does not need a second retirement.
    release_device();
    pending_ = has_frame_;
}

bool SoftwareMovieDisplay::prepare_image(std::uint32_t width, std::uint32_t height,
                                         std::uint32_t format) {
    if (image_ && image_width_ == width && image_height_ == height && image_format_ == format)
        return true;
    const unsigned bytes = pixel_bytes(format);
    if (!width || !height || width > unsigned(INT_MAX) / 4 || height > unsigned(INT_MAX) / 4 ||
        std::uint64_t(width) * bytes * height > std::uint64_t(INT_MAX))
        throw std::runtime_error("MAD invalid backbuffer dimensions");
    if (image_) {
        image_->Release();
        image_ = nullptr;
    }
    horizontal_.resize(width);
    rows_[0].resize(width);
    rows_[1].resize(width);
    for (unsigned x = 0; x < width; ++x)
        horizontal_[x] = display_pixels::sample_axis(x, unsigned(width_), width);
    row_index_[0] = row_index_[1] = -1;
    const HRESULT result = device_->CreateImageSurface(width, height, D3DFORMAT(format), &image_);
    if (needs_recovery(result))
        return false;
    require_success(result, "MAD system-memory image creation failed");
    if (!image_)
        throw std::runtime_error("MAD system-memory image unavailable");
    image_width_ = width;
    image_height_ = height;
    image_format_ = format;
    image_dirty_ = true;
    return true;
}

const std::uint32_t *SoftwareMovieDisplay::scaled_row(unsigned source_row, unsigned slot) {
    if (row_index_[slot] == int(source_row))
        return rows_[slot].data();
    const auto *source = pixels_ + std::size_t(source_row) * unsigned(stride_);
    auto *destination = rows_[slot].data();
    if (image_width_ == unsigned(width_)) {
        std::memcpy(destination, source, std::size_t(image_width_) * 4);
    } else {
        for (unsigned x = 0; x < image_width_; ++x) {
            const auto &sample = horizontal_[x];
            std::uint32_t first, second;
            std::memcpy(&first, source + std::size_t(sample.first) * 4, sizeof(first));
            std::memcpy(&second, source + std::size_t(sample.second) * 4, sizeof(second));
            destination[x] = display_pixels::blend(first, second, sample.weight);
        }
    }
    row_index_[slot] = int(source_row);
    return destination;
}

bool SoftwareMovieDisplay::fill_image() {
    D3DLOCKED_RECT locked{};
    const HRESULT result = image_->LockRect(&locked, nullptr, 0);
    if (needs_recovery(result))
        return false;
    require_success(result, "MAD system-memory image lock failed");
    ImageLock unlock(image_);
    const std::uint64_t row_bytes = std::uint64_t(image_width_) * pixel_bytes(image_format_);
    const std::int64_t pitch = locked.Pitch;
    const std::uint64_t pitch_bytes = std::uint64_t(pitch < 0 ? -pitch : pitch);
    const std::uint64_t span = pitch_bytes * (image_height_ - 1) + row_bytes;
    if (!locked.pBits || pitch_bytes < row_bytes ||
        span > std::uint64_t((std::numeric_limits<std::ptrdiff_t>::max)()))
        throw std::runtime_error("MAD invalid locked image pitch");

    for (unsigned y = 0; y < image_height_; ++y) {
        const auto sample = display_pixels::sample_axis(y, unsigned(height_), image_height_);
        const unsigned first_slot = row_index_[1] == int(sample.first) ? 1 : 0;
        const auto *first = scaled_row(sample.first, first_slot);
        const auto *second = sample.first == sample.second
                                 ? first
                                 : scaled_row(sample.second, 1 - first_slot);
        auto *destination = static_cast<std::uint8_t *>(locked.pBits) +
                            std::ptrdiff_t(y) * std::ptrdiff_t(locked.Pitch);
        write_row(destination, first, second, image_width_, sample.weight, image_format_);
    }
    const HRESULT released = unlock.unlock();
    if (needs_recovery(released))
        return false;
    require_success(released, "MAD system-memory image unlock failed");
    image_dirty_ = false;
    return true;
}

SoftwareMovieDisplay::Result SoftwareMovieDisplay::present() {
    // Consume the request before driver calls: a synchronous paint during those
    // calls must remain pending for the next movie-loop iteration.
    InterlockedExchange(&repaint_, 0);
    auto *current = reinterpret_cast<IDirect3DDevice8 *>(graphics::backend_device);
    if (!current)
        throw std::runtime_error("MAD graphics device unavailable");
    if (device_ != current) {
        release_device();
        device_ = current;
        device_->AddRef();
    }
    const HRESULT cooperative = device_->TestCooperativeLevel();
    if (needs_recovery(cooperative))
        return Result::NeedsRecovery;
    require_success(cooperative, "MAD graphics device is not ready");

    SurfaceReference backbuffer;
    HRESULT result = device_->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer.value);
    if (needs_recovery(result))
        return Result::NeedsRecovery;
    require_success(result, "MAD backbuffer acquisition failed");
    if (!backbuffer.value)
        throw std::runtime_error("MAD backbuffer unavailable");
    D3DSURFACE_DESC description{};
    result = backbuffer.value->GetDesc(&description);
    if (needs_recovery(result))
        return Result::NeedsRecovery;
    require_success(result, "MAD backbuffer description failed");
    // The context selects NONE. Do not change the user's device mode or attempt an undocumented
    // system-memory copy into a multisampled target.
    if (description.MultiSampleType != D3DMULTISAMPLE_NONE)
        throw std::runtime_error("MAD cannot copy to a multisampled backbuffer");
    if (!prepare_image(description.Width, description.Height, description.Format) ||
        (image_dirty_ && !fill_image()))
        return Result::NeedsRecovery;

    // CopyRects requires identical formats/sizes and unlocked surfaces. Copy the
    // complete image even for repaint: DISCARD does not preserve the last buffer.
    result = device_->CopyRects(image_, nullptr, 0, backbuffer.value, nullptr);
    backbuffer.reset(); // Release before Present, message dispatch, or any Reset.
    if (needs_recovery(result))
        return Result::NeedsRecovery;
    require_success(result, "MAD image copy to backbuffer failed");
    result = device_->Present(nullptr, nullptr, nullptr, nullptr);
    if (needs_recovery(result))
        return Result::NeedsRecovery;
    require_success(result, "MAD frame presentation failed");
    pending_ = false;
    return Result::Presented;
}
} // namespace nfs::movies
