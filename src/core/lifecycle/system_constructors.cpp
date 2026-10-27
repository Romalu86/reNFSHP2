#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::lifecycle::construction {
using namespace unresolved;using namespace objects;
static void table(Fields* object,const SystemMethods& methods){*reinterpret_cast<const SystemMethods**>(object)=&methods;}
static std::uint8_t& byte(Fields* object,int offset){return reinterpret_cast<std::uint8_t*>(object)[offset];}
static void pointer(Fields* object,int index,const void* value){object[index]=reinterpret_cast<Dword>(value);}
using ArrayMethod=void(*)(void*);

Fields* net_crypt(Fields* object){table(object,base_system_methods);object[1]=0;table(object,netcryptsystem_methods);return object;}

Fields *fonts_manager(Fields *object) {
    table(object, base_system_methods);
    table(object, fontsmanager_methods);
    auto *fields = static_cast<volatile Dword *>(object);
    fields[1] = 0;
    fields[2] = 0;
    return object;
}

Fields* vehicle_manager(Fields* object){table(object,base_system_methods);vehicle_list(object+1);vehicle_cache(reinterpret_cast<std::uint8_t*>(object)+78600);
    table(object,vehiclemanager_methods);object[20326]=object[20317]=0;byte(object,81296)=1;object[20327]=1120403456;
    byte(object,81297)=1;object[20318]=2;object[20320]=object[20321]=object[20319]=1;object[20328]=1120403456;object[20323]=1;return object;}

Fields* track_manager(Fields* object){table(object,base_system_methods);table(object,trackmanager_methods);object[1]=object[2]=0;
    kernel::memory_fill(object+3,0,128);const char text[]="Invalid";for(int i=0;i<8;++i)byte(object,140+i)=text[i];return object;}

Fields *event_manager(Fields *object) {
    table(object, base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object, base_system_methods); });
    construct_array(object + 2, 36, 100, event_item, event_item_destroy);
    kernel::lifecycle::Cleanup events([&] { destroy_array(object + 2, 36, 100, event_item_destroy); });
    table(object, ceventmanager_methods);
    events.dismiss();
    base.dismiss();
    return object;
}
// Fields 20h..37h are not initialized by this constructor.
Fields* level_manager(Fields* object){table(object,base_system_methods);object[14]=0;table(object,levelmanager_methods);object[1]=0;byte(object,8)=0;
    for(int i=3;i<8;++i)object[i]=0;for(int i=15;i<47;++i)object[i]=0;return object;}
// The element constructor returns its receiver; the destructor has no work.
Fields* world_light(Fields* object){table(object,base_system_methods);table(object,worldlightmanager_methods);byte(object,4)=0;object[2]=0;return object;}

Fields *environment(Fields *object) {
    table(object, base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object, base_system_methods); });
    environment_view(object + 22);
    kernel::lifecycle::Cleanup view([&] { destruction::environment_view(object + 22); });
    environment_target(object + 101);
    kernel::lifecycle::Cleanup target([&] { destruction::environment_target(object + 101); });
    render_state(object + 130);
    kernel::lifecycle::Cleanup state([&] { destruction::render_state_member(object + 130); });
    table(object, enviroreflection_methods);
    object[5] = 0;
    byte(object, 4) = 0;
    object[100] = object[128] = object[129] = object[98] = 0;
    byte(object, 396) = 0;
    state.dismiss();
    target.dismiss();
    view.dismiss();
    base.dismiss();
    return object;
}

Fields *ai_system(Fields *object) {
    table(object, base_system_methods);
    struct ConstructionCleanup {
        Fields *object;
        unsigned completed_members = 0;
        bool finished = false;
        ~ConstructionCleanup() noexcept(false) {
            if (finished)
                return;
            if (completed_members == 2)
                ai_events_destroy(object + 21086);
            if (completed_members >= 1)
                destroy_array(object + 1, 42080, 2, ai_group_destroy);
            table(object, base_system_methods);
        }
    } cleanup{object};
    construct_array(object + 1, 42080, 2, ai_group, ai_group_destroy);
    cleanup.completed_members = 1;
    ai_state(reinterpret_cast<std::uint8_t *>(object) + 84208);
    ai_events(object + 21086);
    cleanup.completed_members = 2;
    table(object, ai_system_methods);
    for (int i = 0; i < 2; ++i)
        ai_group_reset(reinterpret_cast<std::uint8_t *>(object) + 4 + i * 42080);
    object[21041] = object[21042] = object[21043] = 0;
    for (int i = 0; i < 8; ++i)
        object[21044 + i] = 0;
    for (int i = 0; i < 61; ++i)
        object[21098 + i] = 0;
    cleanup.finished = true;
    return object;
}

Fields* car_manager(Fields* object){table(object,base_system_methods);for(int i=1;i<5;++i)object[i]=0;table(object,carmanager_methods);kernel::memory_zero(object+5,96);return object;}
extern Dword speech_handles[10800];

Fields* speech(Fields* object){table(object,base_system_methods);table(object,speech_methods);for(auto& handle:speech_handles)handle=4294967295u;
    for(int i=1;i<=7;++i)object[i]=0;for(int i=9;i<=12;++i)object[i]=0;byte(object,52)=byte(object,53)=0;
    for(int i=14;i<=21;++i)object[i]=0;for(int i=88;i<=94;++i)byte(object,i)=0;return object;}
Dword particle_head=0,particle_tail=0;

Fields* particle_system(Fields* object){table(object,base_system_methods);object[1]=object[2]=object[3]=0;table(object,particlesystem_methods);particle_head=particle_tail=0;return object;}

Fields *skeleton_system(Fields *object) {
    table(object, base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object, base_system_methods); });
    construct_array(object + 4, 96, 256, skeleton_item, skeleton_item_destroy);
    kernel::lifecycle::Cleanup items([&] { destroy_array(object + 4, 96, 256, skeleton_item_destroy); });
    construct_array(object + 6212, 16, 10, skeleton_set, skeleton_set_destroy);
    kernel::lifecycle::Cleanup sets([&] { destroy_array(object + 6212, 16, 10, skeleton_set_destroy); });
    table(object, skeleton_system_methods);
    for (int i = 0; i < 64; ++i)
        object[6148 + i] = 0;
    object[6252] = object[6253] = 0;
    *reinterpret_cast<std::uint16_t *>(object + 6254) = 0;
    sets.dismiss();
    items.dismiss();
    base.dismiss();
    return object;
}

Fields* crc_system(Fields* object,Dword width,Dword polynomial,Dword initial,Dword final_xor,std::uint8_t reflect){table(object,base_system_methods);
    object[257]=width;object[260]=initial;object[259]=polynomial;table(object,crc_system_methods);object[262]=final_xor;object[261]=reflect;return object;}
// Initialize a null pointer; no element cleanup is needed.
Fields* input_system(Fields* object){table(object,base_system_methods);auto* fields=static_cast<volatile Dword*>(object);fields[3]=0;fields[6]=0;for(int i=0;i<52;++i)object[9+i]=0;
    for (int index = 0; index < 32; ++index) {
        byte(object, (253 + index * 2) * 4) = 0;
        object[254 + index * 2] = 0;
    }
    table(object, input_system_methods);
    return object;
}

Fields *shadow_manager(Fields *object) {
    table(object, base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object, base_system_methods); });
    render_state(object + 28);
    kernel::lifecycle::Cleanup state([&] { destruction::render_state_member(object + 28); });
    table(object, shadow_manager_methods);
    identity_matrix(object + 4);
    object[22] = object[21] = object[20] = 0;
    object[23] = 1056964608;
    object[24] = 3156465418u;
    object[25] = object[26] = 1060320051;
    object[27] = 0;
    state.dismiss();
    base.dismiss();
    return object;
}

Fields* platform_system(Fields* object){table(object,base_system_methods);object[1]=0;table(object,platform_system_methods);return object;}
// The callback has no work.
Fields* character_system(Fields* object){table(object,base_system_methods);for(int i=20;i<=23;++i)object[i]=0;
    pointer(object,24,reinterpret_cast<const void*>(character_callback));byte(object,104)=0;object[27]=0;table(object,character_system_methods);return object;}
void unresolved::character_callback(){}

Fields* fog_system(Fields* object){table(object,base_system_methods);table(object,fogsystem_methods);object[17]=255;
    for(int i=18;i<=25;++i)object[i]=0;object[26]=1;byte(object,112)=byte(object,113)=0;return object;}

Fields* replay_system(Fields* object){table(object,base_system_methods);object[1]=0;for(int i=10;i<=13;++i)object[i]=0;table(object,replay_methods);byte(object,8)=0;return object;}

Fields* start_line(Fields* object){table(object,base_system_methods);object[1]=0;table(object,startline_methods);return object;}
Fields* lod_instance=nullptr;
// The settings constructor really is called twice.
Fields* lod_system(Fields* object){table(object,base_system_methods);lod_settings(object+6);table(object,lod_methods);lod_instance=object;
    for(int i=4;i<=7;++i)byte(object,i)=1;object[2]=1036831949;for(int i=12;i<=14;++i)byte(object,i)=1;object[4]=1056964608;byte(object,20)=1;lod_settings(object+6);return object;}

Fields* feedback_system(Fields* object){table(object,base_system_methods);object[1]=object[2]=object[26]=object[50]=4294967295u;byte(object,296)=1;table(object,ffb_methods);
    for(int i=0;i<23;++i){object[3+i]=0;object[27+i]=0;object[51+i]=0;}return object;}
}
