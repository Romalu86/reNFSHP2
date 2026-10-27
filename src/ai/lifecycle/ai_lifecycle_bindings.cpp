#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::lifecycle::table_entries {
void __fastcall reset_skid_pool(System *owner, void *) {
    game::runtime::skids::lifecycle::initialize(reinterpret_cast<Dword *>(owner));
}
void __fastcall ai__initialize(System *owner, void *) {
    nfs::game::runtime::ai::lifecycle::initialize(reinterpret_cast<Dword *>(owner));
}
void __fastcall ai__shutdown(System *owner, void *) {
    nfs::game::runtime::ai::lifecycle::shutdown(reinterpret_cast<Dword *>(owner));
}
void __fastcall ai__load_level(System *owner, void *) {
    nfs::game::runtime::ai::lifecycle::load_level(reinterpret_cast<Dword *>(owner));
}
void __fastcall ai__unload_level(System *owner, void *) {
    nfs::game::runtime::ai::lifecycle::unload_level(reinterpret_cast<Dword *>(owner));
}
void __fastcall ai__reset_level(System *owner, void *) {
    nfs::game::runtime::ai::lifecycle::reset_level(reinterpret_cast<Dword *>(owner));
}
void __fastcall particle_load_level(System *owner, void *) {
    nfs::game::runtime::particles::lifecycle::load_level(reinterpret_cast<Dword *>(owner));
}
void __fastcall particle_unload_level(System *owner, void *) {
    nfs::game::runtime::particles::lifecycle::unload_level(reinterpret_cast<Dword *>(owner));
}
void __fastcall skid_load_level(System *owner, void *) {
    nfs::game::runtime::skids::lifecycle::load_level(reinterpret_cast<Dword *>(owner));
}
void __fastcall skid_unload_level(System *owner, void *) {
    nfs::game::runtime::skids::lifecycle::unload_level(reinterpret_cast<Dword *>(owner));
}
} // namespace nfs::lifecycle::table_entries
