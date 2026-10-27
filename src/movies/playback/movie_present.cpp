#include "movie_runtime.hpp"
#include "movie_software_display.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <new>
#include <stdexcept>
namespace nfs::graphics::backend {
void pump_window_messages(Dword *);
}
namespace nfs::application::pending {
namespace {
struct RestoreMoviePool {
    Dword previous;
    ~RestoreMoviePool() noexcept { kernel::default_pool = previous; }
};

bool movie_quit_requested() {
    MSG quit;
    return movies::stop_requested ||
           PeekMessageA(&quit, nullptr, WM_QUIT, WM_QUIT, PM_NOREMOVE);
}
} // namespace

// The display adapter below is authorized compatibility;
// decoder, frame retirement, audio clock, rate and input providers stay native.
void play_movie(const char *path, Byte skippable, Byte with_audio) {
    namespace movie = nfs::movies;
    SetForegroundWindow(movie::foreground_owner());
    movie::stop_requested = 0;
    if (launch::field<Dword>(8627656))
        with_audio = 0;
    if (launch::field<Dword>(8627676))
        return;
    const RestoreMoviePool restore_pool{kernel::default_pool};
    movie::initialize_memory(&movie::memory);
    movie::memory.pool = lifecycle::track_pool | 1024;
    kernel::default_pool = movie::memory.pool;
    auto *storage =
        static_cast<movie::Player *>(movie::memory.allocate("", 104, 0, 0, movie::memory.pool));
    movie::Player *player = nullptr;
    try {
        player =
            storage ? movie::construct_player(storage, path, 1048576, 0, !with_audio) : nullptr;
    } catch (...) {
        movie::memory.release(storage);
        throw;
    }
    try {
        if (!player)
            throw std::bad_alloc();
        void *frame = movie::start(player, 1, 33);
        if (frame) {
            movie::pause(player);
            movie::SoftwareMovieDisplay display(frame);
            // Start the original clock/audio only after a frame was actually
            // presented. A lost device keeps this CPU copy and pauses that clock.
            bool paused = true;
            bool frame_ready = true;
            Byte skip = 0;
            Dword iterations = 0;
            do {
                graphics::backend::pump_window_messages(graphics::context);
                if (movie_quit_requested())
                    break;
                if (frame_ready) {
                    display.convert(frame);
                    movie::retire_frame(player->delegate, frame);
                    frame_ready = false;
                }
                if (display.needs_present()) {
                    if (display.present() == movie::SoftwareMovieDisplay::Result::NeedsRecovery) {
                        if (!paused) {
                            movie::pause(player);
                            paused = true;
                        }
                        display.prepare_for_recovery();
                        if (!graphics::context ||
                            !graphics::backend::recover_device(graphics::context)) {
                            if (movie_quit_requested())
                                break;
                            throw std::runtime_error("MAD graphics device recovery failed");
                        }
                        // Reacquire the current backbuffer and re-copy the CPU
                        // frame. Decoder-frame ownership was already retired.
                        continue;
                    }
                    if (paused) {
                        movie::resume(player);
                        paused = false;
                    }
                } else {
                    Sleep(1);
                }
                kernel::cooperate(0);
                kernel::thread_yield(0);
                if (movie::frame_due(player)) {
                    frame = movie::seek(player, player->target_frame);
                    if (!frame)
                        break;
                    frame_ready = true;
                }
                if (int(iterations) > 60) {
                    if (skippable)
                        skip = movie::any_input();
                    if (movie::stop_requested)
                        skip = 1;
                }
                ++iterations;
            } while (!skip);
        }
    } catch (...) {
        if (player) {
            movie::destroy_player(player);
            movie::memory.release(player);
        }
        throw;
    }
    if (player) {
        movie::destroy_player(player);
        movie::memory.release(player);
    }
}
} // namespace nfs::application::pending
