#pragma once
#include "../../frontend/screens/frontend_dialogs.hpp"
#include "nfs/input/controllers/controller_resources.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
namespace nfs::profiles {
using namespace application::pending;
using kernel::Dword;
using Byte = std::uint8_t;
constexpr unsigned preferences_size = 50088, file_size = 50272;
struct SavedProfile {
    alignas(4) Byte data[file_size];
};
static_assert(sizeof(SavedProfile) == file_size);
inline void reset_controller_preferences(void *owner) {
    lifecycle::construction::objects::race_slots(owner);
}
void reset_secondary_bindings(void *);
Byte append_controller_preference(void *, Dword, const char *);
void capture_controller_preferences(void *, void *);
inline void reset_progress_record(void *owner) {
    lifecycle::construction::objects::race_statistics(owner);
}
void construct_preferences(void *);
void destroy_preferences(void *);
void reset_preferences(void *);
void copy_profile_dwords(void *, const void *, Dword);
void copy_preferences(void *, const void *);
SavedProfile *construct_profile(SavedProfile *);
SavedProfile *load_profile(SavedProfile *, const char *);
void destroy_profile(SavedProfile *);
void *delete_profile(SavedProfile *, unsigned);
Byte apply_profile(SavedProfile *, int);
void capture_profile(SavedProfile *, int);
Byte save_profile(SavedProfile *, const char *);
void initialize_profile_settings(SavedProfile *, Byte);
void reset_checksum(void *);
void update_checksum(void *, const Byte *, Dword);
Dword profile_checksum(SavedProfile *);
namespace pending {
Byte create_profile(void *, gui::String *);
Byte configure_profile_input();
} // namespace pending
} // namespace nfs::profiles
