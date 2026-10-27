#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdlib>
#include <initializer_list>
namespace nfs::lifecycle::construction::objects {
using W=Dword;
static W* words(void* p){return static_cast<W*>(p);}static std::uint8_t& byte(void* p,int offset){return static_cast<std::uint8_t*>(p)[offset];}
static void table(void* p,const W* methods){words(p)[0]=reinterpret_cast<W>(methods);}
// Each completed element is destroyed in reverse order.




void *event_member(void* p){words(p)[1]=0;table(p,pending::event_member_methods);return p;}

void event_member_destroy(void* p){table(p,pending::event_member_methods);}

void *event_item(void* p){W* object=words(p);object[1]=4294967295u;table(p,pending::input_event_base_methods);
    event_member(object+7);table(p,pending::event_methods);object[7]=reinterpret_cast<W>(pending::event_link_methods);return p;}

void event_item_destroy(void* p){event_member_destroy(words(p)+7);table(p,pending::input_event_base_methods);}

void ai_group_reset(void* p){auto* count=reinterpret_cast<std::uint16_t*>(static_cast<std::uint8_t*>(p)+16856);W* object=words(p);
    for(std::uint16_t i=0;i<*count;++i){auto* child=reinterpret_cast<gui::Object*>(object[3914+i]);if(child)gui::destroy(child);object[3914+i]=0;}
    count[1]=0;count[0]=0;byte(p,16860)=1;}

void *ai_group(void *p) {
    W *object = words(p);
    construct_array(object + 1, 52, 301, ai_node, ai_node_destroy);
    try {
        auto *count = reinterpret_cast<std::uint16_t *>(object + 4214);
        count[0] = 0;
        count[1] = 0;
        byte(p, 16860) = 1;
        object[4216] = 3212836864u;
        ai_bank(object + 4217);
        try {
            table(p, pending::ai_group_methods);
            ai_group_reset(p);
        } catch (...) {
            ai_bank_destroy(object + 4217);
            throw;
        }
    } catch (...) {
        destroy_array(object + 1, 52, 301, ai_node_destroy);
        throw;
    }
    return p;
}

void ai_group_destroy(void *p) {
    table(p, pending::ai_group_methods);
    bool bank_pending = true;
    try {
        ai_group_reset(p);
        bank_pending = false;
        ai_bank_destroy(words(p) + 4217);
    } catch (...) {
        if (bank_pending)
            ai_bank_destroy(words(p) + 4217);
        destroy_array(words(p) + 1, 52, 301, ai_node_destroy);
        throw;
    }
    destroy_array(words(p) + 1, 52, 301, ai_node_destroy);
}

void *ai_events(void* p){W* object=words(p);object[1]=object[10]=0;object[11]=1084227584;table(p,pending::ai_events_methods);for(int i=2;i<10;++i)object[i]=0;return p;}

void ai_events_destroy(void* p){table(p,pending::ai_events_methods);}

void *ai_bank(void *p) {
    W *object = words(p);
    object[1] = 0;
    construct_array(object + 2, 84, 300, ai_bank_item, ai_bank_item_destroy);
    *reinterpret_cast<std::uint16_t *>(object + 6302) = 0;
    table(p, pending::ai_bank_methods);
    return p;
}

void ai_bank_destroy(void* p){table(p,pending::ai_bank_methods);destroy_array(words(p)+2,84,300,ai_bank_item_destroy);}

void *ai_bank_item(void *p) {
    construct_array(words(p) + 3, 8, 8, ai_event_pair, ai_event_pair_destroy);
    *reinterpret_cast<std::uint16_t *>(static_cast<std::uint8_t *>(p) + 76) = 0;
    byte(p, 78) = 0;
    words(p)[20] = 0;
    return p;
}

void ai_bank_item_destroy(void* p){destroy_array(words(p)+3,8,8,ai_event_pair_destroy);}
// BYTE and WORD stores leave gaps in the object untouched.
void race_state(void* p){auto* memory=static_cast<std::uint8_t*>(p);W* object=words(p);
    construct_array(memory+56,4,26,pointer_construct,pointer_destroy);
    for(int start:{8180,28772})for(int i=0;i<33;++i){W* entry=reinterpret_cast<W*>(memory+start+i*624);entry[0]=0;race_statistics(entry+1);}
    for(int i=0;i<3;++i){std::uint8_t* entry=memory+49427+i*68;*reinterpret_cast<W*>(entry+1)=4294967295u;*entry=0;entry[-63]=0;}
    object[49568/4]=0;race_slots(memory+49364);
    for(int i=0;i<32;++i){W* entry=reinterpret_cast<W*>(memory+49580+i*16);entry[-2]=4294967295u;entry[-1]=entry[0]=entry[1]=0;}
    object[50084/4]=0;
    const int clears[][2]={{164,56},{220,24},{244,448},{692,12},{704,12},{716,4128},{4844,48},{4892,344}};
    for(const auto& range:clears)kernel::memory_zero(memory+range[0],range[1]);object[5236/4]=0;
    for(const auto& range:{5240,5288,5632,5976})kernel::memory_zero(memory+range,range==5240?48:344);
    object[6320/4]=0;kernel::memory_zero(memory+6324,48);kernel::memory_zero(memory+6372,344);
    object[6716/4]=object[6720/4]=object[6724/4]=0;kernel::memory_zero(memory+6728,56);kernel::memory_zero(memory+6784,448);
    object[7232/4]=object[7236/4]=object[7240/4]=0;kernel::memory_zero(memory+7244,56);kernel::memory_zero(memory+7300,24);
    object[7324/4]=object[7556/4]=object[7548/4]=object[7552/4]=0;
    kernel::memory_zero(memory+8180,20592);kernel::memory_zero(memory+28772,20592);object[8180/4]=object[28772/4]=1;
    kernel::memory_zero(memory+7328,86);kernel::memory_zero(memory+7462,48);kernel::memory_zero(memory+7414,48);kernel::memory_zero(memory+7512,32);
    for(int offset:{7397,7333,7337,7342,7349,7350,7462,7502})byte(p,offset)=1;object[52/4]=0;
    if(pending::race_seed_enabled)object[7240/4]=std::atoi(pending::race_seed_text);
    object[7512/4]=0;object[7516/4]=9;object[7520/4]=69;object[7524/4]=17;object[7528/4]=9;object[7536/4]=object[7540/4]=0;
    race_statistics(memory+7560);object[160/4]=0;byte(p,7545)=byte(p,7544)=1;}

void race_state_destroy(void* p){destroy_array(static_cast<std::uint8_t*>(p)+56,4,26,pointer_destroy);}

void skeleton_item(void* p){table(p,pending::skeleton_item_methods);words(p)[20]=words(p)[21]=0;identity_matrix(words(p)+4);}

void skeleton_item_destroy(void* p){table(p,pending::skeleton_item_methods);}

void skeleton_set(void* p){table(p,pending::skeleton_set_methods);words(p)[2]=words(p)[3]=words(p)[1]=0;}

void skeleton_set_destroy(void* p){table(p,pending::skeleton_set_methods);}
}
