#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::lifecycle::construction {
void construct_array(void*,int,int,void(*)(void*),void(*)(void*));
// Do not zero the copied word at index 48 or the gaps in the pointer table.
 
Fields* game_rules(Fields* object){using namespace unresolved;using namespace objects;
    *reinterpret_cast<const SystemMethods**>(object)=&base_system_methods;
    kernel::lifecycle::Cleanup base([&] { *reinterpret_cast<const SystemMethods **>(object) = &base_system_methods; });
    for(int i=55;i<=61;++i)object[i]=0;
    for(int i=0;i<24;++i){Fields* entry=object+62+i*21;
        for(int j=0;j<=4;++j)entry[j]=0;entry[5]=4294967295u;entry[14]=1065353216;
        for(int j=15;j<=20;++j)entry[j]=0;reinterpret_cast<std::uint8_t*>(entry)[24]=0;}
    Dword initial=object[48];const int copied[]={47,43,45,44,42,41,40,39,38,37,36,35};
    for(int index:copied)object[index]=initial;object[53]=1;object[566]=0;
    for(int i=567;i<=576;++i)object[i]=3;object[51]=object[52]=4294967295u;
    kernel::memory_zero(object+55,28);kernel::memory_zero(object+62,2016);object[578]=0;object[590]=1;
    for(int i=582;i<=589;++i)object[i]=100;object[591]=0;for(int i=592;i<=605;++i)object[i]=1;
    object[606]=object[607]=0;for(int i=608;i<=612;++i)object[i]=100;for(int i=613;i<=621;++i)object[i]=0;
    construct_array(object+622,50088,2,race_state,race_state_destroy);
    kernel::lifecycle::Cleanup states([&] { destroy_array(object + 622, 50088, 2, race_state_destroy); });
    const int pointers[][2]={
        {25669,38},{25668,37},{25672,55},{25670,41},{25671,39},{25675,58},{25673,56},{25674,57},
        {25678,61},{25676,59},{25677,60},{25681,45},{25679,50},{25680,44},{25685,590},{25682,47},
        {25683,48},{25688,581},{25686,579},{25687,580},{25689,77},{25691,79},{25693,62},{25690,98},
        {25692,100},{25694,83},{25727,621},{25729,25753},{25695,25752},{25699,595},{25700,596},
        {25701,597},{25702,598},{25703,599}};
    for(const auto& pair:pointers)object[pair[0]]=reinterpret_cast<Dword>(object+pair[1]);reinterpret_cast<std::uint8_t*>(object)[102664]=0;
    *reinterpret_cast<const SystemMethods**>(object)=&gamerules_methods;object[25684]=reinterpret_cast<Dword>(object+578);
    object[25696]=reinterpret_cast<Dword>(&graphics_screen_resolution_setting);object[25697]=reinterpret_cast<Dword>(&graphics_colour_depth_setting);
    object[25704]=reinterpret_cast<Dword>(&graphics_gamma_setting);object[25698]=reinterpret_cast<Dword>(&reflection_and_driver_stats_shared_setting);
    const int tail[][2]={{25705,600},{25707,601},{25708,603},{25706,602},{25710,605},{25709,604},
        {25714,594},{25711,591},{25713,593},{25712,592},{25716,583},{25715,582},{25718,589},{25717,588}};
    for(const auto& pair:tail)object[pair[0]]=reinterpret_cast<Dword>(object+pair[1]);object[25728]=reinterpret_cast<Dword>(&reflection_and_driver_stats_shared_setting);
    const int last[][2]={{25725,40},{25719,584},{25720,608},{25721,609},{25726,53},{25723,611},{25724,612},{25722,610}};
    for(const auto& pair:last)object[pair[0]]=reinterpret_cast<Dword>(object+pair[1]);for(int i=3;i<35;++i)object[i]=0;states.dismiss();base.dismiss();return object;}
}
