#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/network/session/network_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include <cstring>
namespace nfs::lifecycle::construction::network {
const std::int32_t game_bases[]={-4,92};
const std::int32_t server_bases[]={-4,316,392};

Dword *thread_construct(Dword* object,const Dword* settings){object[7]=object[8]=0;object[16]=1;object[0]=reinterpret_cast<Dword>(pending::thread_methods);
    object[5]=object[6]=object[15]=0;if(settings){object[1]=settings[0];object[2]=settings[1];object[3]=settings[2];object[4]=settings[3];
        if(settings[3]){if(!object[15])InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));object[15]=1;}}return object;}

void thread_configure(Dword* object,const Dword* settings){if(settings){object[1]=settings[0];object[2]=settings[1];object[3]=settings[2];object[4]=settings[3];
    if(settings[3]){if(!object[15])InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));object[15]=1;}
    else if(object[15]){object[15]=0;DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));}}}

Dword *server_thread(Dword* object){thread_construct(object,nullptr);
    kernel::lifecycle::Cleanup thread([&] { destruction::thread(object); });
    object[17]=reinterpret_cast<Dword>(pending::comparer_interface_methods);
    object[0]=reinterpret_cast<Dword>(pending::server_worker_methods);object[17]=reinterpret_cast<Dword>(pending::server_worker_interface);
    const Dword settings[]={304,0,1,1};
    kernel::lifecycle::Cleanup interface_base([&] { object[17] = reinterpret_cast<Dword>(pending::comparer_interface_methods); });
    thread_configure(object,settings);
    interface_base.dismiss();thread.dismiss();return object;}

Dword *voting_booth(Dword* object){thread_construct(object,nullptr);
    kernel::lifecycle::Cleanup thread([&] { destruction::thread(object); });
    object[17]=reinterpret_cast<Dword>(pending::comparer_interface_methods);object[18]=reinterpret_cast<Dword>(pending::visitor_interface_methods);
    object[0]=reinterpret_cast<Dword>(pending::voting_methods);object[17]=reinterpret_cast<Dword>(pending::voting_first);object[18]=reinterpret_cast<Dword>(pending::voting_second);
    const Dword settings[]={20,0,1,1};
    kernel::lifecycle::Cleanup first([&] { object[17] = reinterpret_cast<Dword>(pending::comparer_interface_methods); });
    kernel::lifecycle::Cleanup second([&] { object[18] = reinterpret_cast<Dword>(pending::visitor_interface_methods); });
    thread_configure(object,settings);object[19]=0;
    second.dismiss();first.dismiss();thread.dismiss();return object;}
}
namespace nfs::lifecycle::construction {
static void table(Dword* object,const SystemMethods& value){object[0]=reinterpret_cast<Dword>(&value);}
static int base_offset(Dword* object,int index){return reinterpret_cast<const std::int32_t*>(object[1])[index];}
static void secondary(Dword* object,int offset,const Dword* methods){*reinterpret_cast<Dword*>(reinterpret_cast<std::uint8_t*>(object)+offset)=reinterpret_cast<Dword>(methods);}

Fields* network_game(Fields* object,int complete){using namespace network;const Dword settings[]={136,0,1,1};
    bool virtual_base_complete = false;
    kernel::lifecycle::Cleanup virtual_base([&] { if (virtual_base_complete) destruction::network_game_base(object + 24); });
    if(complete){object[1]=reinterpret_cast<Dword>(game_bases);thread_construct(object+24,nullptr);
        kernel::lifecycle::Cleanup thread([&] { destruction::thread(object + 24); });
        object[41]=reinterpret_cast<Dword>(pending::comparer_interface_methods);object[42]=reinterpret_cast<Dword>(pending::visitor_interface_methods);
        object[24]=reinterpret_cast<Dword>(pending::game_worker_methods);object[41]=reinterpret_cast<Dword>(pending::game_worker_first);object[42]=reinterpret_cast<Dword>(pending::game_worker_second);kernel::lifecycle::Cleanup first([&] { object[41] = reinterpret_cast<Dword>(pending::comparer_interface_methods); });
        kernel::lifecycle::Cleanup second([&] { object[42] = reinterpret_cast<Dword>(pending::visitor_interface_methods); });
        thread_configure(object+24,settings);
        second.dismiss();first.dismiss();thread.dismiss();virtual_base_complete = true;}
    table(object,base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object,base_system_methods); });
    int offset=base_offset(object,1);table(object,network_game_methods);
    secondary(object,offset+4,pending::game_worker_final);secondary(object,base_offset(object,1)+72,pending::game_first_final);secondary(object,base_offset(object,1)+76,pending::game_second_final);
    *reinterpret_cast<std::uint16_t*>(object+2)=0;*reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uint8_t*>(object)+10)=0;
    object[3]=object[5]=object[6]=object[7]=object[4]=object[8]=object[23]=object[9]=0;for(int i=13;i<19;++i)object[i]=0;
    thread_configure(reinterpret_cast<Dword*>(reinterpret_cast<std::uint8_t*>(object)+base_offset(object,1)+4),settings);
    auto* signal=reinterpret_cast<kernel::Signal*>(object+21);if(kernel::signal_init(signal))kernel::signal_set(signal);object[10]=0;base.dismiss();virtual_base.dismiss();return object;}
// Preserve virtual-base offsets and vtordisp stores.
Fields* network_server(Fields* object,int complete){using namespace network;
    unsigned completed_virtual_bases = 0;
    kernel::lifecycle::Cleanup virtual_bases([&] {
        if (completed_virtual_bases & 2)
            object[99] = reinterpret_cast<Dword>(pending::visitor_interface_methods);
        if (completed_virtual_bases & 1)
            destruction::network_server_base(object + 80);
    });
    if(complete){object[1]=reinterpret_cast<Dword>(server_bases);server_thread(object+80);completed_virtual_bases = 1;object[99]=reinterpret_cast<Dword>(pending::visitor_interface_methods);completed_virtual_bases = 3;}
    table(object,base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object,base_system_methods); });
    for(int i=22;i>=16;--i)object[i]=0;for(int i=76;i>=70;--i)object[i]=0;
    int offset=base_offset(object,1);table(object,server_methods);secondary(object,offset+4,pending::server_worker_final);
    secondary(object,base_offset(object,1)+72,pending::server_first_final);secondary(object,base_offset(object,2)+4,pending::server_second_final);
    *reinterpret_cast<int*>(reinterpret_cast<std::uint8_t*>(object)+base_offset(object,1))=base_offset(object,1)-316;
    *reinterpret_cast<int*>(reinterpret_cast<std::uint8_t*>(object)+base_offset(object,2))=base_offset(object,2)-392;
    auto* signal=reinterpret_cast<kernel::Signal*>(object+4);if(kernel::signal_init(signal))kernel::signal_set(signal);
    const Dword settings[]={304,0,1,1};thread_configure(reinterpret_cast<Dword*>(reinterpret_cast<std::uint8_t*>(object)+base_offset(object,1)+4),settings);
    auto* voting=static_cast<Dword*>(lifecycle::allocate(80,"VotingBooth",lifecycle::level_pool));{
        kernel::lifecycle::Cleanup allocation([&] {
            game::runtime::track::pending::release_named(voting, "VotingBooth", lifecycle::level_pool);
        });
        if(voting)voting = voting_booth(voting);
        allocation.dismiss();
    }
    object[58]=0;*reinterpret_cast<std::uint16_t*>(object+3)=1;object[78]=reinterpret_cast<Dword>(voting);
    object[6]=object[11]=object[12]=object[7]=object[8]=object[9]=object[57]=object[60]=object[59]=0;object[55]=1;
    *reinterpret_cast<std::uint16_t*>(object+56)=1;*reinterpret_cast<std::uint16_t*>(object+13)=8;for(int i=14;i<14+41;++i)object[i]=0;base.dismiss();virtual_bases.dismiss();return object;}

Fields* network_client(Fields* object){table(object,base_system_methods);
    kernel::lifecycle::Cleanup base([&] { table(object,base_system_methods); });
    gui::string_construct(reinterpret_cast<gui::String*>(object+1));
    kernel::lifecycle::Cleanup name([&] { gui::string_destruct(reinterpret_cast<gui::String*>(object+1)); });
    for(int i=109;i<=113;++i)object[i]=0;object[114]=4294967295u;object[123]=1065353216;for(int i=124;i<=129;++i)object[i]=0;
    reinterpret_cast<std::uint8_t*>(object)[460]=0;objects::client_state(object+130);object[14]=object[17]=object[15]=object[523]=object[108]=object[16]=0;
    *reinterpret_cast<std::uint16_t*>(object+524)=0;table(object,client_methods);reinterpret_cast<std::uint8_t*>(object)[2104]=0;
    *reinterpret_cast<std::uint16_t*>(object+95)=1;for(int i=0;i<8;++i){Dword zero=0;std::memcpy(reinterpret_cast<std::uint8_t*>(object)+2105+i*4,&zero,4);}
    for(int i=18;i<18+76;++i)object[i]=0;for(int i=109;i<109+21;++i)object[i]=0;for(int i=130;i<130+393;++i)object[i]=0;
    for(int i=0;i<6;++i){Dword zero=0;std::memcpy(reinterpret_cast<std::uint8_t*>(object)+30+i*4,&zero,4);}
    *reinterpret_cast<std::uint16_t*>(object+94)=0;*reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uint8_t*>(object)+378)=65535;
    object[6]=0;*reinterpret_cast<std::uint16_t*>(object+7)=0;gui::string_copy(reinterpret_cast<gui::String*>(object+1),&unresolved::shared_string);name.dismiss();base.dismiss();return object;}
}
