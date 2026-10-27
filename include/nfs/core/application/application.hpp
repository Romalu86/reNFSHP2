#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/gui/events/gui_tasks.hpp"
#include <cstdio>
#include <initializer_list>

namespace nfs::application {
using kernel::Dword;
using Byte = std::uint8_t;
using GraphicsFailure = Dword(__cdecl *)();
extern char (&executable_path)[260];
extern Byte &os_major_above_four, &windows_2000, &xp_or_later;
extern HANDLE &running_mutex;
extern Byte &shutdown_reached;
extern FILE *null_output;
extern kernel::ErrorHandler &previous_error_handler;
extern GraphicsFailure graphics_failure;
extern Byte cursor_shown;
extern Dword movie_counter;
namespace messages {
extern const wchar_t *const corrupt_files[9];
extern const wchar_t *const graphics_mode[9];
extern const wchar_t *const graphics_hardware[9];
extern const wchar_t *const not_enough_space[9];
extern const wchar_t *const low_disk_space[9];
extern const wchar_t *const error_caption[9];
extern const wchar_t *const directx_required[9];
extern const wchar_t *const unsupported_device[9];
} // namespace messages
int run(int, const char *const *);
int run_outer_flow();
int run_race_screen(void *);
void show_cursor(Byte);
void hide_cursor();
void synchronize_cursor();
void select_movie(char *, Byte);
void seed_effect_random(Dword);
int probe_error_mode();
int read_user_region();
void __cdecl handle_error(const char *, ...);
Dword __cdecl ignore_graphics_failure();
void __cdecl release_console_write(int, const char *);
void set_graphics_failure(GraphicsFailure);
void wait_ticks(int);
Dword tick_frequency();
void prepare_race(void *, Dword);
void notify_race_loaded(void *);
void notify_race_unloading(void *);
void notify_race_unloaded(void *);
void reset_network_race();
void close_loading_screen(void *);
Dword exit_after_network_race();
namespace pending {
extern Dword captured_widget_id;
int exception_filter(EXCEPTION_POINTERS *);
void recover_display();
bool recover_keyboard_hook();
extern char *const exception_text;
void write_memory_report(const char *);
void release_controller_resources();
void play_movie(const char *, Byte, Byte);
int run_initial_frontend(void *);
int run_network_frontend(void *);
int run_frontend(void *, int);
void create_loading_screen(void *);
int switch_display(Dword);
void finish_network_race_session(void *);
Dword create_race(Dword, Dword, const char *, Dword);
void stop_force_feedback();
void reset_subsystems(lifecycle::Manager *);
void reset_race_state();
void clear_frontend_views(void *);
void create_frontend_view(void *, Byte);
int run_starting_grid(void *);
int run_replay_controls(void *);
int run_simulation_screen(void *);
int run_race_results(void *);
int run_end_game_menu(void *);
int run_pursuit_results(void *);
void destroy_screen(void *);
} // namespace pending
} // namespace nfs::application
