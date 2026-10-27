#pragma once
#include "animation_resource_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
Dword __fastcall read_base_animation_kind(void *receiver, void *);
void __fastcall retain_animation_context(void *receiver, void *, Dword time);
Dword __fastcall reject_animation_duration(void *receiver, void *, Dword output);
Dword __fastcall reject_animation_property(void *receiver, void *, Dword time,
                                          Dword output);
Dword __fastcall reject_masked_animation_sample(void *receiver, void *, Dword time, Dword output,
                                               Dword mask);
Dword __fastcall reject_animation_value(void *receiver, void *, Dword time, Dword output);
Dword __fastcall reject_animation_query(void *receiver, void *, Dword time, Dword output);
Dword __fastcall reject_animation_keyframe(void *receiver, void *, Dword time, Dword output, Dword mask,
                                          Dword context);
Dword __fastcall reject_animation_channel_query(void *receiver, void *, Dword time,
                                               Dword output);
Dword __fastcall reject_animation_channel_sample(void *receiver, void *, Dword time,
                                                Dword output);
Dword __fastcall reject_animation_event_query(void *receiver, void *, Dword time, Dword output,
                                             Dword mask);
Dword __fastcall read_empty_animation_owner(void *receiver, void *);
Dword __fastcall read_duration_format_0(void *receiver, void *, Dword output);
Dword __fastcall delete_format_0(void *receiver, void *, Dword flags);
void __fastcall bind_resource_format_1(void *receiver, void *, Dword resource);
Dword __fastcall query_keyframe_format_1(void *receiver, void *, Dword time, Dword output, Dword mask,
                                        Dword context);
Dword __fastcall advance_format_1(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output);
Dword __fastcall delete_format_1(void *receiver, void *, Dword flags);
void __fastcall advance_format_2(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output);
Dword __fastcall read_duration_format_2(void *receiver, void *, Dword output);
Dword __fastcall delete_format_2(void *receiver, void *, Dword flags);
Dword __fastcall delete_format_10(void *receiver, void *, Dword flags);
Dword __fastcall delete_format_11(void *receiver, void *, Dword flags);
void __fastcall advance_base_animation(void *receiver, void *, Dword first_time, Dword second_time,
                                       Dword output);
Dword __fastcall delete_sampled_animation(void *receiver, void *, Dword flags);
Dword __fastcall delete_format_12(void *receiver, void *, Dword flags);
Dword __fastcall delete_format_13(void *receiver, void *, Dword flags);
Dword __fastcall delete_indexed_animation(void *receiver, void *, Dword flags);
Dword __fastcall read_duration_format_10(void *receiver, void *, Dword output);
Dword __fastcall delete_format_5(void *receiver, void *, Dword flags);
Dword __fastcall delete_animation_base(void *receiver, void *, Dword flags);
void __fastcall advance_format_5(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output);
Dword __fastcall delete_format_6(void *receiver, void *, Dword flags);
void __fastcall advance_mirrored_animation(void *receiver, void *, Dword first_time,
                                           Dword second_time,
                                           Dword output);
Dword __fastcall sample_mirrored_pose(void *receiver, void *, Dword time, Dword output,
                                     Dword mask);
Dword __fastcall delete_format_7(void *receiver, void *, Dword flags);
Dword __fastcall delete_format_3(void *receiver, void *, Dword flags);
void __fastcall advance_wrapped_animation(void *receiver, void *, Dword first_time, Dword second_time,
                                          Dword output);
Dword __fastcall query_wrapped_keyframe(void *receiver, void *, Dword time, Dword output, Dword mask,
                                       Dword context);
Dword __fastcall sample_wrapped_pose(void *receiver, void *, Dword time, Dword output,
                                    Dword mask);
Dword __fastcall sample_wrapped_value(void *receiver, void *, Dword time, Dword output);
Dword __fastcall delete_format_4(void *receiver, void *, Dword flags);
Dword __fastcall read_animation_owner(void *receiver, void *);
Dword __fastcall delete_format_14(void *receiver, void *, Dword flags);
Dword __fastcall read_group_animation_kind(void *receiver, void *);
Dword __fastcall read_duration_format_15(void *receiver, void *, Dword output);
Dword __fastcall delete_format_15(void *receiver, void *, Dword flags);
Dword __fastcall read_duration_format_16(void *receiver, void *, Dword output);
Dword __fastcall advance_format_16(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall delete_format_16(void *receiver, void *, Dword flags);
void __fastcall advance_format_0(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output);
Dword __fastcall sample_masked_format_0(void *receiver, void *, Dword time, Dword output,
                                       Dword mask);
void __fastcall advance_transition_animation(void *receiver, void *, Dword first_time,
                                             Dword second_time,
                                             Dword output);
Dword __fastcall sample_blended_pose(void *receiver, void *, Dword time, Dword output,
                                    Dword mask);
void __fastcall advance_blended_animation(void *receiver, void *, Dword first_time, Dword second_time,
                                          Dword output);
void __fastcall bind_resource_format_10(void *receiver, void *, Dword resource);
void __fastcall advance_delta_scalar_animation(void *receiver, void *, Dword first_time,
                                               Dword second_time,
                                               Dword output);
Dword __fastcall sample_delta_scalar_animation(void *receiver, void *, Dword time, Dword output,
                                              Dword mask);
Dword __fastcall query_channel_format_10(void *receiver, void *, Dword time,
                                        Dword output);
Dword __fastcall query_sample_format_10(void *receiver, void *, Dword time, Dword output);
void __fastcall advance_delta_quaternion_animation(void *receiver, void *, Dword first_time,
                                                   Dword second_time,
                                                   Dword output);
Dword __fastcall sample_delta_quaternion_animation(void *receiver, void *, Dword time, Dword output,
                                                  Dword mask);
void __fastcall bind_resource_format_12(void *receiver, void *, Dword resource);
Dword __fastcall read_duration_format_12(void *receiver, void *, Dword output);
Dword __fastcall advance_format_12(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_sparse_delta_scalars(void *receiver, void *, Dword time, Dword output,
                                            Dword mask);
Dword __fastcall advance_format_13(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_sparse_delta_quaternions(void *receiver, void *, Dword time, Dword output,
                                                Dword mask);
Dword __fastcall read_duration_format_14(void *receiver, void *, Dword output);
void __fastcall advance_angle_animation(void *receiver, void *, Dword first_time, Dword second_time,
                                        Dword output);
void __fastcall bind_resource_format_14(void *receiver, void *, Dword resource);
void __fastcall bind_resource_format_15(void *receiver, void *, Dword resource);
Dword __fastcall read_group_animation_flags(void *receiver, void *);
void __fastcall advance_format_15(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall query_keyframe_format_15(void *receiver, void *, Dword time, Dword output, Dword mask,
                                         Dword context);
Dword __fastcall sample_masked_format_15(void *receiver, void *, Dword time, Dword output,
                                        Dword mask);
Dword __fastcall query_channel_format_15(void *receiver, void *, Dword time,
                                        Dword output);
Dword __fastcall query_sample_format_15(void *receiver, void *, Dword time, Dword output);
Dword __fastcall sample_channel_format_15(void *receiver, void *, Dword time,
                                         Dword output);
Dword __fastcall query_event_format_15(void *receiver, void *, Dword time, Dword output,
                                      Dword mask);
Dword __fastcall sample_value_format_15(void *receiver, void *, Dword time, Dword output);
Dword __fastcall read_owner_format_15(void *receiver, void *);
void __fastcall set_group_animation_context(void *receiver, void *, Dword time);
Dword __fastcall delete_resource_animation(void *receiver, void *, Dword flags);
void __fastcall bind_animation_resource(void *receiver, void *, Dword resource);
Dword __fastcall animation_resource_address(void *receiver, void *);
Dword __fastcall animation_resource(void *receiver, void *);
Dword __fastcall read_resource_animation_kind(void *receiver, void *);
Dword __fastcall read_empty_animation_flags(void *receiver, void *);
Dword __fastcall sample_record_animation(void *receiver, void *, Dword time,
                                        Dword output);
Dword __fastcall find_record_animation_event(void *receiver, void *, Dword time, Dword output,
                                            Dword mask);
Dword __fastcall delete_format_21(void *receiver, void *, Dword flags);
void __fastcall bind_resource_format_21(void *receiver, void *, Dword resource);
Dword __fastcall read_duration_format_21(void *receiver, void *, Dword output);
Dword __fastcall advance_format_21(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_block_scalar_animation(void *receiver, void *, Dword time, Dword output,
                                              Dword mask);
Dword __fastcall query_channel_format_21(void *receiver, void *, Dword time,
                                        Dword output);
Dword __fastcall query_sample_format_21(void *receiver, void *, Dword time, Dword output);
Dword __fastcall delete_format_20(void *receiver, void *, Dword flags);
void __fastcall bind_resource_format_20(void *receiver, void *, Dword resource);
Dword __fastcall read_duration_format_20(void *receiver, void *, Dword output);
Dword __fastcall advance_format_20(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_block_vector_animation(void *receiver, void *, Dword time, Dword output,
                                              Dword mask);
Dword __fastcall query_channel_format_20(void *receiver, void *, Dword time,
                                        Dword output);
Dword __fastcall query_sample_format_20(void *receiver, void *, Dword time, Dword output);
Dword __fastcall delete_format_19(void *receiver, void *, Dword flags);
void __fastcall bind_resource_format_19(void *receiver, void *, Dword resource);
Dword __fastcall read_duration_format_19(void *receiver, void *, Dword output);
Dword __fastcall advance_format_19(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_masked_format_19(void *receiver, void *, Dword time, Dword output,
                                        Dword mask);
Dword __fastcall sample_axis_rotation_pose(void *receiver, void *, Dword time, Dword mask,
                                          Dword output);
Dword __fastcall delete_format_18(void *receiver, void *, Dword flags);
void __fastcall bind_compact_rotation_animation(void *receiver, void *, Dword resource);
Dword __fastcall read_duration_format_18(void *receiver, void *, Dword output);
Dword __fastcall advance_format_18(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_compact_rotation_pose(void *receiver, void *, Dword time, Dword output,
                                             Dword mask);
Dword __fastcall delete_format_17(void *receiver, void *, Dword flags);
void __fastcall bind_resource_format_17(void *receiver, void *, Dword resource);
Dword __fastcall read_duration_format_17(void *receiver, void *, Dword output);
Dword __fastcall advance_format_17(void *receiver, void *, Dword first_time, Dword second_time,
                                  Dword output);
Dword __fastcall sample_masked_format_17(void *receiver, void *, Dword time, Dword output,
                                        Dword mask);
Dword __fastcall sample_three_component_rotation_pose(void *receiver, void *, Dword time, Dword mask,
                                                     Dword output);
Dword __fastcall delete_format_9(void *receiver, void *, Dword flags);
Dword __fastcall sample_duration_blended_pose(void *receiver, void *, Dword time, Dword output,
                                             Dword mask);
Dword __fastcall advance_format_9(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output);
Dword __fastcall reject_format_9_value(void *receiver, void *, Dword time, Dword output);
Dword __fastcall sample_duration_blended_displacement(void *receiver, void *, Dword time,
                                                     Dword output);
Dword __fastcall delete_format_8(void *receiver, void *, Dword flags);
Dword __fastcall sample_frame_blended_pose(void *receiver, void *, Dword time, Dword output,
                                          Dword mask);
Dword __fastcall advance_format_8(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output);
Dword __fastcall reject_format_8_value(void *receiver, void *, Dword time, Dword output);
Dword __fastcall sample_frame_blended_displacement(void *receiver, void *, Dword time,
                                                  Dword output);
Dword __fastcall query_transition_crossing(void *receiver, void *, Dword time,
                                          Dword output);
extern const Dword format_0_methods[18];
extern const Dword format_1_methods[18];
extern const Dword format_2_methods[18];
extern const Dword format_3_methods[15];
extern const Dword format_4_methods[15];
extern const Dword format_5_methods[15];
extern const Dword format_6_methods[15];
extern const Dword format_7_methods[15];
extern const Dword format_8_methods[15];
extern const Dword format_9_methods[15];
extern const Dword format_10_methods[18];
extern const Dword format_11_methods[18];
extern const Dword format_12_methods[18];
extern const Dword format_13_methods[18];
extern const Dword format_14_methods[18];
extern const Dword format_15_methods[18];
extern const Dword format_16_methods[18];
extern const Dword format_17_methods[19];
extern const Dword format_18_methods[18];
extern const Dword format_19_methods[19];
extern const Dword format_20_methods[18];
extern const Dword format_21_methods[18];
extern const Dword resource_animation_methods[18];
extern const Dword indexed_animation_methods[18];
extern const Dword sampled_animation_methods[18];
extern const Dword animation_base_methods[15];
void *construct_resource_animation(void *);
void destroy_resource_animation(void *);
void *allocate_animation(Dword);
void recycle_buffer(Dword *);
Dword *allocate_animation_buffer(Dword);
} // namespace nfs::game::world::actor_resources::animation_runtime
