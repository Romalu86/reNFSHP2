#include "runtime_animation_dispatch.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include <cstring>
namespace nfs::game::world::actor_resources::animation_runtime {

Dword __fastcall read_base_animation_kind(void *, void *) {
    return 0;
}

Dword __fastcall reject_animation_duration(void *, void *, Dword) {
    return 0;
}

Dword __fastcall reject_animation_property(void *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_masked_animation_sample(void *, void *, Dword, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_animation_value(void *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_animation_query(void *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_animation_keyframe(void *, void *, Dword, Dword, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_animation_channel_query(void *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_animation_channel_sample(void *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_animation_event_query(void *, void *, Dword, Dword, Dword) {
    return 0;
}

Dword __fastcall read_empty_animation_owner(void *, void *) {
    return 0;
}

Dword __fastcall read_empty_animation_flags(void *, void *) {
    return 0;
}

Dword __fastcall reject_format_9_value(void *, void *, Dword, Dword) {
    return 0;
}

Dword __fastcall reject_format_8_value(void *, void *, Dword, Dword) {
    return 0;
}

void __fastcall retain_animation_context(void *, void *, Dword) {
}

void __fastcall advance_base_animation(void *, void *, Dword, Dword, Dword) {
}

void __fastcall bind_animation_resource(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
}

Dword __fastcall animation_resource_address(void *receiver, void *) {
    return at<Dword>(receiver, 12);
}

Dword __fastcall animation_resource(void *receiver, void *) {
    return at<Dword>(receiver, 12);
}

Dword __fastcall read_animation_owner(void *receiver, void *) {
    return at<Dword>(receiver, 12);
}

Dword __fastcall read_resource_animation_kind(void *receiver, void *) {
    Dword resource = at<Dword>(receiver, 12);
    return (resource & 4294901760u) | at<std::uint16_t>(reinterpret_cast<void *>(resource), 2);
}

Dword __fastcall read_group_animation_kind(void *receiver, void *) {
    Dword resource = at<Dword>(receiver, 12);
    return (resource & 4294901760u) | at<std::uint16_t>(reinterpret_cast<void *>(resource), 2);
}

Dword __fastcall read_group_animation_flags(void *receiver, void *) {
    return at<Dword>(at<void *>(receiver, 12), 4);
}

void __fastcall bind_resource_format_1(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
    at<Dword>(receiver, 16) = at<Dword>(receiver, 20) = 0;
}

void __fastcall bind_resource_format_14(void *receiver, void *, Dword resource) {
    at<std::uint16_t>(receiver, 16) = 0;
    at<Dword>(receiver, 12) = resource;
    void *data = reinterpret_cast<void *>(resource);
    at<std::uint16_t>(receiver, 18) = at<std::uint16_t>(data, 6);
    Byte flags = at<Byte>(data, 8);
    at<Byte>(receiver, 20) = ~flags & 1;
    at<Byte>(receiver, 21) = (flags & 8)    ? 1
                             : (flags & 16) ? 2
                             : (flags & 32) ? 4
                             : (flags & 64) ? 8
                                            : 1;
}

void __fastcall bind_resource_format_19(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
}

void __fastcall bind_resource_format_17(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
}

void __fastcall bind_resource_format_21(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
    at<Dword>(receiver, 16) = at<Dword>(receiver, 28) = ~0u;
}

void __fastcall bind_resource_format_20(void *receiver, void *, Dword resource) {
    at<Dword>(receiver, 12) = resource;
    at<Dword>(receiver, 16) = at<Dword>(receiver, 28) = ~0u;
}

Dword __fastcall read_duration_format_0(void *receiver, void *, Dword output) {
    Dword resource = at<Dword>(receiver, 12);
    *reinterpret_cast<float *>(output) = float(at<int>(reinterpret_cast<void *>(resource), 12));
    return (resource & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_2(void *receiver, void *, Dword output) {
    Dword resource = at<Dword>(receiver, 12);
    *reinterpret_cast<float *>(output) =
        float(at<std::uint16_t>(reinterpret_cast<void *>(resource), 6));
    return (resource & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_10(void *receiver, void *, Dword output) {
    Dword resource = at<Dword>(receiver, 12);
    *reinterpret_cast<float *>(output) =
        float(at<std::uint16_t>(reinterpret_cast<void *>(resource), 8));
    return (resource & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_16(void *receiver, void *, Dword output) {
    Dword resource = at<Dword>(receiver, 12);
    *reinterpret_cast<float *>(output) =
        float(at<std::uint16_t>(reinterpret_cast<void *>(resource), 4));
    return (resource & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_14(void *receiver, void *, Dword output) {
    Dword resource = at<Dword>(receiver, 12);
    *reinterpret_cast<float *>(output) =
        float(at<std::uint16_t>(reinterpret_cast<void *>(resource), 4));
    return (resource & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_15(void *receiver, void *, Dword output) {
    void *data = at<void *>(receiver, 12);
    float duration = float(at<std::uint16_t>(data, 10));
    *reinterpret_cast<float *>(output) = duration;
    if (at<Byte>(receiver, 20))
        *reinterpret_cast<float *>(output) = duration / float(at<Byte>(receiver, 21));
    return 1;
}

Dword __fastcall read_duration_format_19(void *receiver, void *, Dword output) {
    Dword result = at<Dword>(receiver, 12);
    void *data = reinterpret_cast<void *>(result);
    if (at<Dword>(data, 8))
        result =
            Dword(at<std::uint16_t>(at<Byte *>(data, 8), Dword(at<std::uint16_t>(data, 4)) * 2 - 4)) +
            1;
    *reinterpret_cast<float *>(output) =
        at<Dword>(data, 8) ? float(result) : float(at<std::uint16_t>(data, 4));
    return (result & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_18(void *receiver, void *, Dword output) {
    Dword result = at<Dword>(receiver, 12);
    void *data = reinterpret_cast<void *>(result);
    if (at<Dword>(data, 8))
        result =
            Dword(at<std::uint16_t>(at<Byte *>(data, 8), Dword(at<std::uint16_t>(data, 4)) * 2 - 4)) +
            1;
    *reinterpret_cast<float *>(output) =
        at<Dword>(data, 8) ? float(result) : float(at<std::uint16_t>(data, 4));
    return (result & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_17(void *receiver, void *, Dword output) {
    Dword result = at<Dword>(receiver, 12);
    void *data = reinterpret_cast<void *>(result);
    if (at<Dword>(data, 8))
        result =
            Dword(at<std::uint16_t>(at<Byte *>(data, 8), Dword(at<std::uint16_t>(data, 4)) * 2 - 4)) +
            1;
    *reinterpret_cast<float *>(output) =
        at<Dword>(data, 8) ? float(result) : float(at<std::uint16_t>(data, 4));
    return (result & 4294967040u) | 1u;
}

Dword __fastcall read_duration_format_21(void *receiver, void *, Dword output) {
    void *data = at<void *>(receiver, 12);
    std::uint16_t duration = at<std::uint16_t>(data, 12);
    if (at<Dword>(data, 8))
        duration = std::uint16_t(at<std::int16_t>(at<Byte *>(data, 8), Dword(duration) * 2 - 4) + 1);
    *reinterpret_cast<float *>(output) = float(duration);
    return 1;
}

Dword __fastcall read_duration_format_20(void *receiver, void *, Dword output) {
    void *data = at<void *>(receiver, 12);
    std::uint16_t duration = at<std::uint16_t>(data, 12);
    if (at<Dword>(data, 8))
        duration = std::uint16_t(at<std::int16_t>(at<Byte *>(data, 8), Dword(duration) * 2 - 4) + 1);
    *reinterpret_cast<float *>(output) = float(duration);
    return 1;
}

Dword __fastcall read_duration_format_12(void *receiver, void *, Dword output) {
    void *data = at<void *>(receiver, 12);
    Dword duration =
        Dword(at<std::uint16_t>(at<Byte *>(data, 8), Dword(at<std::uint16_t>(data, 12)) * 2 - 4)) + 1;
    *reinterpret_cast<float *>(output) = float(duration);
    return (duration & 4294967040u) | 1u;
}

Dword __fastcall advance_format_12(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall advance_format_13(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall advance_format_21(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall advance_format_20(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall advance_format_18(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall advance_format_9(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall advance_format_8(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 6, second_time, output, 0u);
}

Dword __fastcall query_channel_format_10(void *receiver, void *, Dword time, Dword output) {
    world::invoke(receiver, 3, time, time, output);
    return 1;
}

Dword __fastcall query_sample_format_10(void *receiver, void *, Dword time, Dword output) {
    world::invoke(receiver, 3, time, time, output);
    return 1;
}

Dword __fastcall query_channel_format_21(void *receiver, void *, Dword time, Dword output) {
    world::invoke(receiver, 3, time, time, output);
    return 1;
}

Dword __fastcall query_sample_format_21(void *receiver, void *, Dword time, Dword output) {
    world::invoke(receiver, 3, time, time, output);
    return 1;
}

Dword __fastcall query_channel_format_20(void *receiver, void *, Dword time, Dword output) {
    world::invoke(receiver, 3, time, time, output);
    return 1;
}

Dword __fastcall query_sample_format_20(void *receiver, void *, Dword time, Dword output) {
    world::invoke(receiver, 3, time, time, output);
    return 1;
}

Dword __fastcall advance_format_19(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 18, second_time, 0u, output);
}

Dword __fastcall advance_format_17(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 18, second_time, 0u, output);
}

Dword __fastcall sample_masked_format_19(void *receiver, void *, Dword time, Dword output, Dword mask) {
    return world::invoke<Dword>(receiver, 18, time, mask, output);
}

Dword __fastcall sample_masked_format_17(void *receiver, void *, Dword time, Dword output, Dword mask) {
    return world::invoke<Dword>(receiver, 18, time, mask, output);
}

Dword __fastcall advance_format_1(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output) {
    return world::invoke<Dword>(receiver, 9, first_time, second_time, output, 0u);
}

Dword __fastcall advance_format_16(void *receiver, void *, Dword, Dword second_time, Dword output) {
    return world::invoke<Dword>(receiver, 11, second_time, output);
}

void __fastcall advance_format_5(void *receiver, void *, Dword first_time, Dword second_time,
                                 Dword output) {
    for (int index = 0; index < at<int>(receiver, 16); ++index)
        world::invoke(at<void **>(receiver, 12)[index], 3, first_time, second_time, output);
}
const Dword format_0_methods[18] = {reinterpret_cast<Dword>(delete_format_0),
                                   reinterpret_cast<Dword>(read_resource_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_format_0),
                                   reinterpret_cast<Dword>(read_duration_format_0),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(sample_masked_format_0),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags),
                                   reinterpret_cast<Dword>(bind_animation_resource),
                                   reinterpret_cast<Dword>(animation_resource),
                                   reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_1_methods[18] = {reinterpret_cast<Dword>(delete_format_1),
                                   reinterpret_cast<Dword>(read_resource_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_format_1),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(reject_masked_animation_sample),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(query_keyframe_format_1),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags),
                                   reinterpret_cast<Dword>(bind_resource_format_1),
                                   reinterpret_cast<Dword>(animation_resource),
                                   reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_2_methods[18] = {reinterpret_cast<Dword>(delete_format_2),
                                   reinterpret_cast<Dword>(read_resource_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_format_2),
                                   reinterpret_cast<Dword>(read_duration_format_2),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(reject_masked_animation_sample),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags),
                                   reinterpret_cast<Dword>(bind_animation_resource),
                                   reinterpret_cast<Dword>(animation_resource),
                                   reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_3_methods[15] = {reinterpret_cast<Dword>(delete_format_3),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_wrapped_animation),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(sample_wrapped_pose),
                                   reinterpret_cast<Dword>(sample_wrapped_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(query_wrapped_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword format_4_methods[15] = {reinterpret_cast<Dword>(delete_format_4),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_transition_animation),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(reject_masked_animation_sample),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword format_5_methods[15] = {reinterpret_cast<Dword>(delete_format_5),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_format_5),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(reject_masked_animation_sample),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword format_6_methods[15] = {reinterpret_cast<Dword>(delete_format_6),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_blended_animation),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(sample_blended_pose),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword format_7_methods[15] = {reinterpret_cast<Dword>(delete_format_7),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_mirrored_animation),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(sample_mirrored_pose),
                                   reinterpret_cast<Dword>(reject_animation_value),
                                   reinterpret_cast<Dword>(reject_animation_query),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword format_10_methods[18] = {
    reinterpret_cast<Dword>(delete_format_10),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_delta_scalar_animation),
    reinterpret_cast<Dword>(read_duration_format_10),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_delta_scalar_animation),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(query_sample_format_10),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(query_channel_format_10),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_10),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_11_methods[18] = {
    reinterpret_cast<Dword>(delete_format_11),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_delta_quaternion_animation),
    reinterpret_cast<Dword>(read_duration_format_10),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_delta_quaternion_animation),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_10),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_12_methods[18] = {
    reinterpret_cast<Dword>(delete_format_12),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_12),
    reinterpret_cast<Dword>(read_duration_format_12),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_sparse_delta_scalars),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_12),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_13_methods[18] = {
    reinterpret_cast<Dword>(delete_format_13),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_13),
    reinterpret_cast<Dword>(read_duration_format_12),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_sparse_delta_quaternions),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_12),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_14_methods[18] = {
    reinterpret_cast<Dword>(delete_format_14),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_angle_animation),
    reinterpret_cast<Dword>(read_duration_format_14),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(reject_masked_animation_sample),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_14),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_15_methods[18] = {
    reinterpret_cast<Dword>(delete_format_15),
    reinterpret_cast<Dword>(read_group_animation_kind),
    reinterpret_cast<Dword>(set_group_animation_context),
    reinterpret_cast<Dword>(advance_format_15),
    reinterpret_cast<Dword>(read_duration_format_15),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_masked_format_15),
    reinterpret_cast<Dword>(sample_value_format_15),
    reinterpret_cast<Dword>(query_sample_format_15),
    reinterpret_cast<Dword>(query_keyframe_format_15),
    reinterpret_cast<Dword>(query_channel_format_15),
    reinterpret_cast<Dword>(sample_channel_format_15),
    reinterpret_cast<Dword>(query_event_format_15),
    reinterpret_cast<Dword>(read_owner_format_15),
    reinterpret_cast<Dword>(read_group_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_15),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_16_methods[18] = {
    reinterpret_cast<Dword>(delete_format_16),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_16),
    reinterpret_cast<Dword>(read_duration_format_16),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(reject_masked_animation_sample),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(sample_record_animation),
    reinterpret_cast<Dword>(find_record_animation_event),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_animation_resource),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_21_methods[18] = {
    reinterpret_cast<Dword>(delete_format_21),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_21),
    reinterpret_cast<Dword>(read_duration_format_21),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_block_scalar_animation),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(query_sample_format_21),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(query_channel_format_21),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_21),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_20_methods[18] = {
    reinterpret_cast<Dword>(delete_format_20),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_20),
    reinterpret_cast<Dword>(read_duration_format_20),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_block_vector_animation),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(query_sample_format_20),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(query_channel_format_20),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_20),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_19_methods[19] = {reinterpret_cast<Dword>(delete_format_19),
                                    reinterpret_cast<Dword>(read_resource_animation_kind),
                                    reinterpret_cast<Dword>(retain_animation_context),
                                    reinterpret_cast<Dword>(advance_format_19),
                                    reinterpret_cast<Dword>(read_duration_format_19),
                                    reinterpret_cast<Dword>(reject_animation_property),
                                    reinterpret_cast<Dword>(sample_masked_format_19),
                                    reinterpret_cast<Dword>(reject_animation_value),
                                    reinterpret_cast<Dword>(reject_animation_query),
                                    reinterpret_cast<Dword>(reject_animation_keyframe),
                                    reinterpret_cast<Dword>(reject_animation_channel_query),
                                    reinterpret_cast<Dword>(reject_animation_channel_sample),
                                    reinterpret_cast<Dword>(reject_animation_event_query),
                                    reinterpret_cast<Dword>(read_empty_animation_owner),
                                    reinterpret_cast<Dword>(read_empty_animation_flags),
                                    reinterpret_cast<Dword>(bind_resource_format_19),
                                    reinterpret_cast<Dword>(animation_resource),
                                    reinterpret_cast<Dword>(animation_resource_address),
                                    reinterpret_cast<Dword>(sample_axis_rotation_pose)};
const Dword format_18_methods[18] = {
    reinterpret_cast<Dword>(delete_format_18),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_18),
    reinterpret_cast<Dword>(read_duration_format_18),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_compact_rotation_pose),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_compact_rotation_animation),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword format_17_methods[19] = {
    reinterpret_cast<Dword>(delete_format_17),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_format_17),
    reinterpret_cast<Dword>(read_duration_format_17),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(sample_masked_format_17),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_17),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address),
    reinterpret_cast<Dword>(sample_three_component_rotation_pose)};
const Dword format_9_methods[15] = {reinterpret_cast<Dword>(delete_format_9),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_format_9),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(reject_animation_property),
                                   reinterpret_cast<Dword>(sample_duration_blended_pose),
                                   reinterpret_cast<Dword>(reject_format_9_value),
                                   reinterpret_cast<Dword>(sample_duration_blended_displacement),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword format_8_methods[15] = {reinterpret_cast<Dword>(delete_format_8),
                                   reinterpret_cast<Dword>(read_base_animation_kind),
                                   reinterpret_cast<Dword>(retain_animation_context),
                                   reinterpret_cast<Dword>(advance_format_8),
                                   reinterpret_cast<Dword>(reject_animation_duration),
                                   reinterpret_cast<Dword>(query_transition_crossing),
                                   reinterpret_cast<Dword>(sample_frame_blended_pose),
                                   reinterpret_cast<Dword>(reject_format_8_value),
                                   reinterpret_cast<Dword>(sample_frame_blended_displacement),
                                   reinterpret_cast<Dword>(reject_animation_keyframe),
                                   reinterpret_cast<Dword>(reject_animation_channel_query),
                                   reinterpret_cast<Dword>(reject_animation_channel_sample),
                                   reinterpret_cast<Dword>(reject_animation_event_query),
                                   reinterpret_cast<Dword>(read_empty_animation_owner),
                                   reinterpret_cast<Dword>(read_empty_animation_flags)};
const Dword resource_animation_methods[18] = {
    reinterpret_cast<Dword>(delete_resource_animation),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_base_animation),
    reinterpret_cast<Dword>(reject_animation_duration),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(reject_masked_animation_sample),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_animation_resource),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword indexed_animation_methods[18] = {
    reinterpret_cast<Dword>(delete_indexed_animation),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_base_animation),
    reinterpret_cast<Dword>(read_duration_format_10),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(reject_masked_animation_sample),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_10),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword sampled_animation_methods[18] = {
    reinterpret_cast<Dword>(delete_sampled_animation),
    reinterpret_cast<Dword>(read_resource_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_base_animation),
    reinterpret_cast<Dword>(read_duration_format_12),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(reject_masked_animation_sample),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags),
    reinterpret_cast<Dword>(bind_resource_format_12),
    reinterpret_cast<Dword>(animation_resource),
    reinterpret_cast<Dword>(animation_resource_address)};
const Dword animation_base_methods[15] = {
    reinterpret_cast<Dword>(delete_animation_base),
    reinterpret_cast<Dword>(read_base_animation_kind),
    reinterpret_cast<Dword>(retain_animation_context),
    reinterpret_cast<Dword>(advance_base_animation),
    reinterpret_cast<Dword>(reject_animation_duration),
    reinterpret_cast<Dword>(reject_animation_property),
    reinterpret_cast<Dword>(reject_masked_animation_sample),
    reinterpret_cast<Dword>(reject_animation_value),
    reinterpret_cast<Dword>(reject_animation_query),
    reinterpret_cast<Dword>(reject_animation_keyframe),
    reinterpret_cast<Dword>(reject_animation_channel_query),
    reinterpret_cast<Dword>(reject_animation_channel_sample),
    reinterpret_cast<Dword>(reject_animation_event_query),
    reinterpret_cast<Dword>(read_empty_animation_owner),
    reinterpret_cast<Dword>(read_empty_animation_flags)};
} // namespace nfs::game::world::actor_resources::animation_runtime
