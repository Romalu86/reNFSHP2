#pragma once
#include "../../profiles/storage/player_profile.hpp"
#include "nfs/input/directinput/directinput.hpp"
namespace nfs::controllers {
using namespace application::pending;
void *get_definitions();
void *get_default_maps();
int load_default_maps(void *);
int load_definitions(void *);
void *keyboard_controller();
void *find_definition(void *, const char *);
void *find_default_map(void *, const char *);
Byte add_controller(void *, const char *, Dword);
void *controller_at(void *, Dword);
Byte matches_saved_controller(void *, Dword, const void *);
void select_primary_controller(void *, int);
void clear_device_assignments(void *);
void enable_device_assignment(void *, Dword, Byte);
char *lower_controller_name(char *);
int pov_direction(int);
const char *action_name(int);
int key_from_name(const char *);
struct InputAction {
    const char *name;
    int action;
};
extern const InputAction input_actions[25];
} // namespace nfs::controllers
