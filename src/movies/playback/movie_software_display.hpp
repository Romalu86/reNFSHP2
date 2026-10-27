#pragma once
#include "movie_cpu_pixels.hpp"
#include <windows.h>
#include <cstdint>
#include <vector>

struct IDirect3DDevice8;
struct IDirect3DSurface8;

namespace nfs::movies {
// Authorized MAD display compatibility. Decode and colour conversion remain on
// the CPU. The existing game device presents a system-memory image; there is no
// second display owner, video texture, drawing scene, or device-mode transition.
class SoftwareMovieDisplay {
  public:
    enum class Result { Presented, NeedsRecovery };
    explicit SoftwareMovieDisplay(void *first_frame);
    ~SoftwareMovieDisplay() noexcept;
    SoftwareMovieDisplay(const SoftwareMovieDisplay &) = delete;
    SoftwareMovieDisplay &operator=(const SoftwareMovieDisplay &) = delete;
    void convert(void *frame);
    bool needs_present() const noexcept;
    Result present();
    void prepare_for_recovery() noexcept;

  private:
    using Redraw = void(__cdecl *)();
    static SoftwareMovieDisplay *active_;
    static void __cdecl request_redraw() noexcept;
    SoftwareMovieDisplay *previous_active_ = nullptr;
    Redraw previous_redraw_ = nullptr;
    bool owns_redraw_ = false;
    volatile LONG repaint_ = 0;
    void *allocation_ = nullptr;
    std::uint8_t *pixels_ = nullptr;
    int width_ = 0, height_ = 0, stride_ = 0;
    bool has_frame_ = false, pending_ = false, image_dirty_ = true;
    IDirect3DDevice8 *device_ = nullptr;
    IDirect3DSurface8 *image_ = nullptr;
    std::uint32_t image_width_ = 0, image_height_ = 0, image_format_ = 0;
    std::vector<display_pixels::AxisSample> horizontal_;
    std::vector<std::uint32_t> rows_[2];
    int row_index_[2] = {-1, -1};

    bool prepare_image(std::uint32_t width, std::uint32_t height, std::uint32_t format);
    bool fill_image();
    const std::uint32_t *scaled_row(unsigned source_row, unsigned slot);
    void release_device() noexcept;
    void release() noexcept;
};
} // namespace nfs::movies
