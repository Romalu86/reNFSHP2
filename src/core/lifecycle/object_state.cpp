#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <initializer_list>
namespace nfs::lifecycle::construction::objects {
using W=Dword;
static W* words(void* p){return static_cast<W*>(p);}static std::uint8_t& byte(void* p,int offset){return static_cast<std::uint8_t*>(p)[offset];}

void vehicle_part(void* p){words(p)[2]=0;}
// The 20 part constructors run before the later bulk clear.
void vehicle_list(void* p){W* item=words(p)+1;for(int slot=0;slot<64;++slot,item+=307){
    for(int i=0;i<20;++i)vehicle_part(item+16+i*12);
    item[259]=item[272]=0;item[10]=item[9]=item[5]=item[4]=item[14]=item[13]=item[12]=item[11]=0;
    item[0]=4294967295u;item[15]=0;kernel::memory_zero(item+16,960);byte(item,1148)=byte(item,1076)=byte(item,24)=byte(item,32)=0;item[282]=0;
    for(int i=0;i<20;++i){W* part=item+20+i*12;part[1]=part[2]=200;part[-4]=4294967295u;byte(part,0)=0;
        part[3]=220;part[4]=1086324736;part[5]=part[6]=1114636288;part[7]=1101004800;}
    item[306]=1077936128;item[305]=1109393408;item[304]=1061158912;}}

void vehicle_cache(void* p){W* object=words(p);for(int i=0;i<12;++i){W* item=object+3+i*10;
    for(int j=3;j<=7;++j)item[j]=0;item[0]=item[1]=item[2]=0;item[8]=4294967295u;item[9]=0;}
    for(int start:{123,395})for(int i=0;i<68;++i){W* item=object+start+i*4;item[0]=item[1]=0;item[2]=4294967295u;item[3]=0;}
    object[0]=object[1]=object[2]=0;}

void* track_state(void* p){W* object=words(p);object[22]=object[7]=object[21]=object[5]=object[0]=object[4]=object[1]=object[2]=object[3]=object[6]=object[20]=0;return p;}

void* track_item(void* p){W* object=words(p);object[39]=object[0]=object[17]=object[19]=object[40]=object[41]=object[1]=object[2]=object[6]=0;return p;}
 
void track_item_destroy(void*){}

void environment_view(void* p){W* object=words(p);object[2]=object[1]=object[6]=object[5]=object[4]=0;object[3]=1065353216;
    for(int i=7;i<=11;++i)object[i]=1112014848;object[12]=1068149419;object[13]=1065353216;object[14]=1181376512;
    object[17]=object[15]=0;object[16]=1065353216;object[18]=4294967295u;}

void environment_target(void* p){W* object=words(p);object[0]=object[2]=object[3]=object[4]=object[1]=0;byte(p,100)=0;}

void *ai_state(void* p){W* object=words(p);object[1]=0;byte(p,132)=0;object[2]=0;
    for(int i=0;i<10;++i){W* item=object+5+i*3;item[-2]=0;item[-1]=30;item[0]=0;}return p;}
// The constant vector is exactly {0, 0, 0}.
void* ai_node(void* p){W* object=words(p);*reinterpret_cast<std::uint16_t*>(object+11)=0;*reinterpret_cast<std::uint16_t*>(object+11)=0;
    object[0]=object[1]=object[2]=0;return p;}

void ai_node_destroy(void*){}

void* ai_event_pair(void* p){words(p)[0]=0;words(p)[1]=0;return p;}

void ai_event_pair_destroy(void*){}

void race_statistics(void* p){W* object=words(p);object[0]&=4294967054u;kernel::memory_zero(object+77,192);for(int i=0;i<12;++i)object[137+i]=0;}

void race_slots(void* p){W* object=words(p);object[51]=0;for(int i=0;i<3;++i)object[16+i*17]=4294967295u;}

void vehicle_pose(void* p){W* object=words(p);for(int i=0;i<=8;++i)object[i]=0;object[9]=object[10]=object[11]=1065353216;
    kernel::memory_zero(object+14,104);kernel::memory_zero(object+40,104);object[68]=0;object[66]=1065353216;object[67]=1053609165;
    object[12]=3189509823u;object[13]=1033637567;}

void vehicle_dynamics(void* p){W* object=words(p);object[3]=object[2]=object[1]=object[0]=0;object[4]=1066192077;
    for(int i=71;i>=68;--i)byte(p,i)=0;object[14]=object[13]=object[18]=object[16]=object[15]=0;
    for(int i=12;i>=5;--i)object[i]=0;object[19]=object[20]=1065353216;byte(p,84)=0;
    for(int i=26;i>=22;--i)object[i]=0;object[28]=object[27]=0;for(int i=38;i>=29;--i)object[i]=0;
    object[67]=1065353216;object[40]=object[39]=object[68]=0;kernel::memory_zero(object+41,4);kernel::memory_zero(object+42,100);
    byte(p,276)=0;object[101]=object[99]=object[71]=object[70]=object[100]=object[98]=object[97]=object[96]=0;
    object[102]=1063675494;for(int i=72;i<96;++i)object[i]=0;vehicle_pose(object+103);vehicle_pose(object+172);
    for(int i=241;i<=249;++i)object[i]=0;}

void client_state(void* p){W* object=words(p);for(int i=0;i<=36;++i)object[i]=0;
    kernel::memory_zero(object+37,32);kernel::memory_zero(object+45,32);kernel::memory_zero(object+53,32);
    for(int i=61;i<=67;++i)object[i]=0;kernel::memory_zero(object+68,32);kernel::memory_zero(object+76,32);
    object[85]=object[84]=object[86]=0;kernel::memory_zero(object+87,8);for(int i=89;i<=92;++i)object[i]=0;
    const W identity[]={1065353216,0,0,0,1065353216,0,0,0,1065353216};for(int i=0;i<9;++i){object[93+i]=identity[i];object[102+i]=identity[i];}
    for(int i=111;i<=123;++i)object[i]=0;vehicle_dynamics(object+124);object[390]=object[391]=object[392]=0;}
}

namespace nfs::lifecycle::construction {
using Fields=kernel::Dword;

void identity_matrix(Fields* object){for(int i=0;i<16;++i)object[i]=i%5?0:1065353216;}

void lod_settings(Fields* object){object[0]=object[1]=object[2]=1065353216;object[3]=1133903872;object[4]=1134723072;
    object[5]=1202702336;object[6]=1204702336;object[7]=3173242634u;}
}
