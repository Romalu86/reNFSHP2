#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::lifecycle::construction::objects {
using kernel::Dword;
void vehicle_part(void*);void vehicle_list(void*);void vehicle_cache(void*);
void *track_state(void*);void *track_item(void*);void track_item_destroy(void*);
void environment_view(void*);void environment_target(void*);void *ai_state(void*);
void *ai_node(void*);void ai_node_destroy(void*);void *ai_event_pair(void*);void ai_event_pair_destroy(void*);
void race_statistics(void*);void race_slots(void*);void vehicle_pose(void*);void vehicle_dynamics(void*);void client_state(void*);
void *event_member(void*);void event_member_destroy(void*);void *event_item(void*);void event_item_destroy(void*);
void *ai_group(void*);void ai_group_destroy(void*);void ai_group_reset(void*);void *ai_events(void*);void ai_events_destroy(void*);
void *ai_bank(void*);void ai_bank_destroy(void*);void *ai_bank_item(void*);void ai_bank_item_destroy(void*);
void race_state(void*);void race_state_destroy(void*);void skeleton_item(void*);void skeleton_item_destroy(void*);
void skeleton_set(void*);void skeleton_set_destroy(void*);
void destroy_array(void*,int,int,void(*)(void*));
void *pointer_construct(void*);void pointer_destroy(void*);
namespace pending {
extern const Dword input_event_base_methods[],event_methods[],event_link_methods[],event_member_methods[];
extern const Dword ai_group_methods[],ai_events_methods[],ai_bank_methods[];
extern const Dword skeleton_item_methods[],skeleton_set_methods[];
extern Dword &race_seed_enabled;extern char (&race_seed_text)[24];
}
}
