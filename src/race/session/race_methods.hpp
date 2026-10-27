#pragma once
#include "race_runtime.hpp"
namespace nfs::application::pending {
struct GridCarPool;
void __fastcall tick_event_eleven(void *race, void *);
void *__fastcall clone_event_eleven(void *race, void *);
void __fastcall event_eleven_collision(void *race, void *, Dword attacker, Dword victim,
                                       Dword);
Byte __fastcall paired_can_finish(void *, void *, Dword);
Byte __fastcall paired_has_pairs(void *, void *);
void *__fastcall delete_event_eleven(void *race, void *, unsigned flags);
void __fastcall race_frame_hook(void *, void *);
void __fastcall race_owner_hook(void *, void *);
void __fastcall race_after_start_hook(void *, void *);
void __fastcall race_post_reset_hook(void *, void *);
void __fastcall race_pre_step_hook(void *, void *);
void __fastcall race_elimination_hook(void *, void *, Dword);
Dword __fastcall race_eliminated_count(void *, void *);
Byte __fastcall race_completion_hook(void *, void *);
void __fastcall tick_quota_event(void *race, void *);
void *__fastcall clone_event_six(void *race, void *);
void __fastcall quota_arrest(void *race, void *, Dword attacker, Dword victim, Dword);
void __fastcall quota_collision(void *race, void *, Dword victim, Dword attacker,
                                Dword severity);
int __fastcall quota_next_option(void *, void *, int current);
void __fastcall quota_checkpoint_hook(void *, void *, Dword, Dword, Dword);
Byte __fastcall quota_has_pairs(void *, void *);
void *__fastcall delete_event_six(void *race, void *, unsigned flags);
Byte __fastcall event_complete(void *race, void *);
void *__fastcall clone_event_base(void *race, void *);
void *__fastcall delete_event_base(void *race, void *, unsigned flags);
void __fastcall tick_event_twelve(void *race, void *);
void *__fastcall clone_event_twelve(void *race, void *);
void __fastcall event_twelve_arrest(void *race, void *, Dword, Dword victim, Dword);
Byte __fastcall event_twelve_has_pairs(void *, void *);
void *__fastcall delete_event_twelve(void *race, void *, unsigned flags);
void __fastcall tick_event_five(void *race, void *);
void *__fastcall clone_event_five(void *race, void *);
Byte __fastcall event_five_can_finish(void *, void *, Dword);
void *__fastcall delete_event_five(void *race, void *, unsigned flags);
void __fastcall tick_event_thirteen(void *race, void *);
void *__fastcall clone_event_thirteen(void *race, void *);
void __fastcall event_thirteen_takedown(void *race, void *, Dword slot, Dword, Dword);
void *__fastcall delete_event_thirteen(void *race, void *, unsigned flags);
void __fastcall elimination_checkpoint(void *race, void *, Dword slot, int completed,
                                       Dword time);
int __fastcall elimination_racer_count(void *race, void *);
void __fastcall elimination_arrest(void *race, void *, Dword, Dword victim, Dword);
void __fastcall tick_elimination_event(void *race, void *);
void *__fastcall clone_event_one(void *race, void *);
Byte __fastcall elimination_complete(void *race, void *);
void __fastcall reset_elimination_race(void *race, void *);
Byte __fastcall elimination_last_position(void *race, void *, Dword slot,
                                          int checkpoint);
Byte __fastcall elimination_has_active_player(void *race, void *, Byte local_only);
int __fastcall elimination_remaining_count(void *race, void *);
int __fastcall elimination_next_option(void *, void *, int current);
void __fastcall eliminate_participant(void *race, void *, Dword slot);
void __fastcall initialize_elimination_options(void *race, void *);
Dword __fastcall elimination_count(void *race, void *);
void *__fastcall delete_event_one(void *race, void *, unsigned flags);
Byte __fastcall timed_event_complete(void *race, void *);
void *__fastcall clone_event_seven(void *race, void *);
int __fastcall timed_participant_remaining(void *race, void *, Dword slot);
void __fastcall timed_checkpoint(void *race, void *, Dword slot, int checkpoint,
                                 Dword time);
int __fastcall timed_next_option(void *, void *, int current);
Byte __fastcall timed_completion_hook(void *, void *);
void *__fastcall delete_event_seven(void *race, void *, unsigned flags);
void __fastcall tick_event_nine(void *race, void *);
void *__fastcall clone_event_nine(void *race, void *);
void __fastcall paired_event_takedown(void *race, void *, Dword slot, Dword, Dword);
void __fastcall paired_checkpoint_hook(void *, void *, Dword, Dword, Dword);
int __fastcall paired_next_option(void *, void *, int current);
void *__fastcall delete_event_nine(void *race, void *, unsigned flags);
void __fastcall tick_pursuit_event(void *race, void *);
void __fastcall pursuit_takedown(void *race, void *, Dword slot, Dword, Dword);
void *__fastcall clone_event_four(void *race, void *);
int __fastcall pursuit_next_option(void *, void *, int current);
void __fastcall pursuit_checkpoint_hook(void *, void *, Dword, Dword, Dword);
Byte __fastcall pursuit_has_pairs(void *, void *);
Byte __fastcall pursuit_can_finish(void *, void *, Dword);
void *__fastcall delete_event_four(void *race, void *, unsigned flags);
void *__fastcall clone_race_state(void *race, void *);
void __fastcall reset_race_statistics(void *race, void *);
void __fastcall initialize_race_options(void *race, void *);
void __fastcall begin_race_statistics(void *race, void *);
void __fastcall begin_race_run(void *race, void *);
Byte __fastcall race_has_pairs(void *, void *);
void __fastcall configure_race_population(void *race, void *);
void __fastcall prepare_grid_car_pool(void *, void *, GridCarPool *pool, Dword racers, int cops,
                                      int selection);
void __fastcall select_participant_grid(void *race, void *, Dword slot);
void __fastcall assign_grid_car(void *, void *, void *choices, Dword slot);
void __fastcall assign_pursuit_grid_car(void *, void *, void *choices, Dword slot,
                                        Byte alternate);
void __fastcall configure_race_grid(void *race, void *, Dword retain_grid);
void __fastcall request_race_finish(void *race, void *, Dword delay);
void __fastcall finish_race_run(void *race, void *);
void __fastcall display_race_results(void *race, void *);
int __fastcall evaluate_race_medal(void *race, void *, const int *thresholds, void *record,
                                   Byte cop);
void __fastcall update_race_results(void *race, void *);
void __fastcall tick_race_deadline(void *race, void *);
Byte __fastcall race_base_complete(void *, void *);
Byte __fastcall participant_last_position(void *race, void *, Dword slot,
                                          int checkpoint);
Byte __fastcall participant_can_continue(void *race, void *, Dword slot);
void __fastcall record_checkpoint(void *race, void *, Dword slot, int completed,
                                  Dword time);
void __fastcall record_collision(void *race, void *, Dword attacker, Dword victim, Dword);
void __fastcall race_collision_score(void *race, void *, Dword, Dword victim,
                                     Dword severity);
int __fastcall count_remaining_racers(void *race, void *);
int __fastcall count_local_racers(void *race, void *);
Byte __fastcall race_has_active_player(void *race, void *, Byte local_only);
Byte __fastcall race_uses_rewards(void *race, void *);
void __fastcall sort_participant_positions(void *race, void *, Dword *positions,
                                           int *count);
void __fastcall sort_racer_positions(void *race, void *, Dword *positions, int *count);
void __fastcall sort_cop_positions(void *race, void *, Dword *positions, int *count);
int __fastcall participant_position(void *race, void *, Dword slot);
int __fastcall participant_elapsed_time(void *race, void *, Dword slot);
int __fastcall participant_current_split(void *race, void *, Dword slot);
Dword __fastcall elapsed_race_ticks(void *, void *);
Byte __fastcall participant_reached_final_lap(void *race, void *, Dword slot);
int __fastcall participant_remaining_time(void *race, void *, Dword slot);
int __fastcall race_next_option(void *, void *, int current);
void __fastcall write_race_stat_text(void *race, void *, Dword statistic, Dword slot,
                                     void *output);
void *__fastcall delete_race_state(void *race, void *, unsigned flags);
void *__fastcall delete_event_two(void *race, void *, unsigned flags);
void *__fastcall clone_event_zero(void *race, void *);
void __fastcall event_zero_arrest(void *race, void *, Dword, Dword victim, Dword);
void *__fastcall delete_event_zero(void *race, void *, unsigned flags);
void __fastcall tick_paired_event(void *race, void *);
void *__fastcall clone_paired_event(void *race, void *);
void __fastcall configure_paired_grid(void *race, void *, Dword settings);
void *__fastcall delete_paired_event(void *race, void *, unsigned flags);
void __fastcall tick_time_trial(void *race, void *);
void *__fastcall clone_event_three(void *race, void *);
void __fastcall time_trial_checkpoint(void *race, void *, Dword slot, int checkpoint,
                                      Dword time);
int __fastcall time_trial_next_option(void *, void *, int current);
Dword __fastcall time_trial_elapsed(void *race, void *);
Byte __fastcall time_trial_uses_rewards(void *, void *);
void __fastcall reset_time_trial(void *race, void *);
void *__fastcall delete_event_three(void *race, void *, unsigned flags);
Byte __fastcall event_two_has_pairs(void *, void *);
void *__fastcall clone_event_two(void *race, void *);
void __fastcall reset_event_two_progress(void *race, void *);
void __fastcall configure_event_two_grid(void *race, void *, Dword);
void __fastcall event_two_collision(void *race, void *, Dword attacker, Dword victim,
                                    Dword severity);
} // namespace nfs::application::pending
