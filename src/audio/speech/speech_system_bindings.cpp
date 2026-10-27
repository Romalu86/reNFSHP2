#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::lifecycle::table_entries {
void __fastcall shutdown_speech(System *owner, void *) {
    speech::game::shutdown_owner(owner);
}
void __fastcall load_speech_level(System *owner, void *) {
    speech::game::load_level(owner);
}
void __fastcall unload_speech_level(System *owner, void *) {
    speech::game::unload_level(owner);
}
void __fastcall reset_speech_race(System *owner, void *) {
    speech::game::reset_race(owner);
}
void __fastcall enter_speech_frontend(System *owner, void *) {
    speech::game::enter_frontend(owner);
}
void __fastcall leave_speech_frontend(System *owner, void *) {
    speech::game::leave_frontend(owner);
}
void __fastcall post_initialize_speech(System *owner, void *) {
    speech::game::post_initialize(owner);
}
} // namespace nfs::lifecycle::table_entries
