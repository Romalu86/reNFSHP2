#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/network/session/network_construction.hpp"
namespace nfs::lifecycle::construction::destruction {
static void table(Dword* object,const SystemMethods& methods){object[0]=reinterpret_cast<Dword>(&methods);}
static void dispose(Dword value){if(value)gui::destroy(reinterpret_cast<gui::Object*>(value));}
// Inlined String member destruction has a separate Object-base unwind action.
static void destroy_string_member(gui::String *value) {
    kernel::lifecycle::Cleanup base([&] { value->methods = gui::base_methods; });
    gui::string_destruct(value);
    base.dismiss();
}

void dedicated_console(Dword* p){table(p,base_system_methods);}

void net_crypt(Dword* p){table(p,netcryptsystem_methods);table(p,base_system_methods);}

void fonts_manager(Dword* p){table(p,fontsmanager_methods);table(p,base_system_methods);}

void vehicle_cache(Dword*){}

void vehicle_list(Dword*){}

void vehicle_manager(Dword* p){table(p,vehiclemanager_methods);vehicle_cache(p+19650);vehicle_list(p+1);table(p,base_system_methods);}

void track_manager(Dword* p){table(p,base_system_methods);}

void event_manager(Dword *p) {
    table(p, ceventmanager_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    objects::destroy_array(p + 2, 36, 100, objects::event_item_destroy);
}

void level_manager(Dword* p){table(p,base_system_methods);}

void track_state(Dword*){}

void track(Dword *p) {
    table(p, track_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    kernel::lifecycle::Cleanup state([&] { track_state(p + 19); });
    objects::destroy_array(p + 42, 168, 128, objects::track_item_destroy);
}

void light_record(Dword*){}

void world_light(Dword* p){table(p,worldlightmanager_methods);reinterpret_cast<std::uint8_t*>(p)[4]=0;
    for(int i=3;i>0;)light_record(reinterpret_cast<Dword*>(reinterpret_cast<std::uint8_t*>(p)+76+--i*28));table(p,base_system_methods);}

void render_state(Dword*){}

void render_state_member(Dword*){}

void environment_target(Dword*){}

void environment_view(Dword* p){p[0]=0;}

void environment(Dword *p) {
    table(p, enviroreflection_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    kernel::lifecycle::Cleanup view([&] { environment_view(p + 22); });
    kernel::lifecycle::Cleanup target([&] { environment_target(p + 101); });
    render_state_member(p + 130);
}

void ai_system(Dword *p) {
    table(p, ai_system_methods);
    struct BaseCleanup {
        Dword *object;
        ~BaseCleanup() { table(object, base_system_methods); }
    } base{p};
    struct GroupsCleanup {
        Dword *object;
        ~GroupsCleanup() noexcept(false) {
            objects::destroy_array(object + 1, 42080, 2, objects::ai_group_destroy);
        }
    } groups{p};
    objects::ai_events_destroy(p + 21086);
}

void game_rules(Dword *p) {
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    objects::destroy_array(p + 622, 50088, 2, objects::race_state_destroy);
}

void car_manager(Dword* p){table(p,base_system_methods);}

void audio(Dword* p){table(p,base_system_methods);}

void speech(Dword* p){table(p,base_system_methods);}

void audio_loader(Dword* p){table(p,base_system_methods);}

void particle_system(Dword* p){table(p,particlesystem_methods);table(p,base_system_methods);}
// These owned objects delete through slot 84h, not slot zero.
void race_manager(Dword *p) {
    table(p, racemanager_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    using Delete = void(__thiscall *)(void *, unsigned);
    for (int i = 1; i <= 2; ++i)
        if (p[i]) {
            auto *object = reinterpret_cast<gui::Object *>(p[i]);
            reinterpret_cast<const Delete *>(object->methods)[132 / 4](object, 1);
        }
}

void skeleton_system(Dword *p) {
    table(p, skeleton_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    kernel::lifecycle::Cleanup items([&] {
        objects::destroy_array(p + 4, 96, 256, objects::skeleton_item_destroy);
    });
    objects::destroy_array(p + 6212, 16, 10, objects::skeleton_set_destroy);
}

void crc_system(Dword* p){table(p,base_system_methods);}
// Element destruction has no effects; the pointer array is not released.
void input_system(Dword* p){table(p,input_system_methods);table(p,base_system_methods);}

void shadow_manager(Dword *p) {
    table(p, shadow_manager_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    render_state_member(p + 28);
}

void platform_system(Dword* p){table(p,platform_system_methods);table(p,base_system_methods);}

void path_system(Dword* p){table(p,base_system_methods);}
static int base_offset(Dword* p,int index){return reinterpret_cast<const std::int32_t*>(p[1])[index];}
static void secondary(Dword* p,int offset,const Dword* methods){*reinterpret_cast<Dword*>(reinterpret_cast<std::uint8_t*>(p)+offset)=reinterpret_cast<Dword>(methods);}

void network_game(Dword *p) {
    table(p, network_game_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    secondary(p, base_offset(p, 1) + 4, network::pending::game_worker_final);
    secondary(p, base_offset(p, 1) + 72, network::pending::game_first_final);
    secondary(p, base_offset(p, 1) + 76, network::pending::game_second_final);
    kernel::signal_destroy(reinterpret_cast<kernel::Signal *>(p + 21));
}

void network_server(Dword *p) {
    table(p, server_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    secondary(p, base_offset(p, 1) + 4, network::pending::server_worker_final);
    secondary(p, base_offset(p, 1) + 72, network::pending::server_first_final);
    secondary(p, base_offset(p, 2) + 4, network::pending::server_second_final);
    *reinterpret_cast<int *>(reinterpret_cast<std::uint8_t *>(p) + base_offset(p, 1)) =
        base_offset(p, 1) - 316;
    *reinterpret_cast<int *>(reinterpret_cast<std::uint8_t *>(p) + base_offset(p, 2)) =
        base_offset(p, 2) - 392;
    dispose(p[78]);
    kernel::signal_destroy(reinterpret_cast<kernel::Signal *>(p + 4));
}

void network_client(Dword *p) {
    table(p, client_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    destroy_string_member(reinterpret_cast<gui::String *>(p + 1));
}
// Double enter/leave and each flag re-read are intentional.
void thread(Dword* p){p[0]=reinterpret_cast<Dword>(network::pending::thread_methods);auto* lock=reinterpret_cast<CRITICAL_SECTION*>(p+9);
    if(p[15]){EnterCriticalSection(lock);if(p[15])EnterCriticalSection(lock);}Dword item=p[5];
    using ReleasePayload=void(__thiscall*)(Dword*,Dword);
    while(item){auto* node=reinterpret_cast<Dword*>(item);Dword next=node[1];reinterpret_cast<const ReleasePayload*>(p[0])[7](p,node[4]);gui::application_release(node);item=next;}
    p[7]=0;p[16]=1;p[5]=p[6]=0;if(p[15]){LeaveCriticalSection(lock);if(p[15]){LeaveCriticalSection(lock);if(p[15]){p[15]=0;DeleteCriticalSection(lock);}}}}
// Receives the worker base at complete-object offset 60h.
void network_game_base(Dword* p){secondary(p,72,network::pending::visitor_interface_methods);secondary(p,68,network::pending::comparer_interface_methods);thread(p);}
// Receives the worker base at complete-object offset 140h.
void network_server_base(Dword* p){secondary(p,68,network::pending::comparer_interface_methods);thread(p);}

void skid_system(Dword* p){render_state_member(p+5);table(p,base_system_methods);}

void camera_system(Dword* p){table(p,base_system_methods);}

void character_system(Dword *object) {
    table(object, character_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object, base_system_methods); });
    const int count = static_cast<int>(object[21]);
    if (count > 0) {
        auto *entries = reinterpret_cast<Dword *>(object[22]);
        for (int index = 0; index < count; ++index)
            gui::application_release(reinterpret_cast<void *>(entries[index]));
    }
    // The array owner is sampled again after the element releases.
    if (object[21])
        gui::application_release(reinterpret_cast<void *>(object[22]));
    const Dword resource = static_cast<volatile Dword *>(object)[23];
    static_cast<volatile Dword *>(object)[21] = 0;
    if (resource) {
        dispose(resource);
        object[23] = 0;
    }
    if (object[27]) {
        dispose(object[27]);
        object[27] = 0;
    }
}

void fog_system(Dword* p){table(p,fogsystem_methods);table(p,base_system_methods);}
// The container at 54h only resets its base vptr here.
void frontend(Dword *p) {
    table(p, frontend_methods);
    kernel::lifecycle::Cleanup base([&] { table(p, base_system_methods); });
    kernel::lifecycle::Cleanup screen([&] { destroy_string_member(reinterpret_cast<gui::String *>(p + 4)); });
    p[21] = reinterpret_cast<Dword>(gui::base_methods);
    destroy_string_member(reinterpret_cast<gui::String *>(p + 14));
}

void replay(Dword* p){table(p,replay_methods);table(p,base_system_methods);}

void start_line(Dword* p){table(p,startline_methods);if(p[1]){dispose(p[1]);p[1]=0;}table(p,base_system_methods);}

void lod_system(Dword* p){table(p,lod_methods);table(p,base_system_methods);}

void feedback_system(Dword* p){table(p,ffb_methods);table(p,base_system_methods);}
}
