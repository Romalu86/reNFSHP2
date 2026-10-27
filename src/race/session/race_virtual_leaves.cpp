#include "race_runtime.hpp"
namespace nfs::application::pending {
 

Byte __fastcall paired_can_finish(void *, void *, Dword) {
    return 0;
}

Byte __fastcall paired_has_pairs(void *, void *) {
    return 1;
}

void __fastcall race_frame_hook(void *, void *) {
}

void __fastcall race_owner_hook(void *, void *) {
}

void __fastcall race_after_start_hook(void *, void *) {
}

void __fastcall race_post_reset_hook(void *, void *) {
}

void __fastcall race_pre_step_hook(void *, void *) {
}

void __fastcall race_elimination_hook(void *, void *, Dword) {
}

Dword __fastcall race_eliminated_count(void *, void *) {
    return 0;
}

Byte __fastcall race_completion_hook(void *, void *) {
    return 0;
}

void __fastcall quota_checkpoint_hook(void *, void *, Dword, Dword, Dword) {
}

Byte __fastcall quota_has_pairs(void *, void *) {
    return 1;
}

Byte __fastcall event_twelve_has_pairs(void *, void *) {
    return 1;
}

Byte __fastcall event_five_can_finish(void *, void *, Dword) {
    return 0;
}

Byte __fastcall timed_completion_hook(void *, void *) {
    return 1;
}

void __fastcall paired_checkpoint_hook(void *, void *, Dword, Dword, Dword) {
}

void __fastcall pursuit_checkpoint_hook(void *, void *, Dword, Dword, Dword) {
}

Byte __fastcall pursuit_has_pairs(void *, void *) {
    return 1;
}

Byte __fastcall pursuit_can_finish(void *, void *, Dword) {
    return 0;
}

Byte __fastcall race_has_pairs(void *, void *) {
    return 0;
}

Byte __fastcall race_base_complete(void *, void *) {
    return 0;
}

Byte __fastcall time_trial_uses_rewards(void *, void *) {
    return 1;
}

Byte __fastcall event_two_has_pairs(void *, void *) {
    return 0;
}
} // namespace nfs::application::pending
