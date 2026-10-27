#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {

void assign_animation_property_key(std::uint16_t *key, std::uint16_t value) {
    *key = value;
}

void copy_animation_property_key(std::uint16_t *key, const std::uint16_t *source) {
    *key = *source;
}

void *find_animation_property(void *properties, std::uint16_t key) {
    int lower = 0, upper = int(Dword(at<int>(properties, 0)) - 1);
    if (upper < 0)
        return nullptr;
    while (lower <= upper) {
        int middle = int(Dword(upper) + Dword(lower)) >> 1;
        auto *entry = offset(properties, 4 + Dword(middle) * 8);
        std::uint16_t candidate = at<std::uint16_t>(entry, 0);
        if (candidate < key)
            lower = int(Dword(middle) + 1);
        else if (candidate <= key)
            return entry;
        else
            upper = int(Dword(middle) - 1);
    }
    return nullptr;
}

Dword read_animation_property(void *properties, std::uint16_t key, Dword *value) {
    if (void *entry = find_animation_property(properties, key)) {
        Dword result = at<Dword>(entry, 4);
        *value = result;
        return (result & 4294967040u) | 1;
    }
    return 0;
}

void __fastcall set_group_animation_context(void *receiver, void *, Dword enabled) {
    at<Byte>(receiver, 20) = Byte(enabled);
    if (at<Byte>(receiver, 21) || !Byte(enabled))
        return;
    std::uint16_t argument_key;
    assign_animation_property_key(&argument_key, 1);
    void *properties = world::invoke<void *>(receiver, 14);
    if (!properties)
        return;
    std::uint16_t local_key, forwarded_key;
    copy_animation_property_key(&local_key, &argument_key);
    copy_animation_property_key(&forwarded_key, &local_key);
    Dword rate;
    if (Byte(read_animation_property(properties, forwarded_key, &rate)))
        at<Byte>(receiver, 21) = Byte(rate);
}

void decode_channel_record(void *owner, const Byte *cursor, Byte *output) {
    void *resource = at<void *>(owner, 12);
    Dword value = 0;
    Byte consumed_bits = 0;
    for (Dword index = 0; index < at<Byte>(resource, 8); ++index) {
        std::uint16_t field = at<std::uint16_t>(resource, 10 + index * 2);
        Byte destination_width = Byte(field >> 11) & 3;
        switch (field >> 13) {
        case 0:
            consumed_bits = Byte(consumed_bits + 1);
            value = (*cursor >> ((8 - consumed_bits) & 31)) & 1;
            break;
        case 1:
            consumed_bits = Byte(consumed_bits + 2);
            value = (*cursor >> ((8 - consumed_bits) & 31)) & 3;
            break;
        case 2:
            consumed_bits = Byte(consumed_bits + 4);
            value = (*cursor >> ((8 - consumed_bits) & 31)) & 15;
            break;
        case 3:
            value = *cursor++;
            break;
        case 4:
            value = at<std::uint16_t>(cursor, 0);
            cursor += 2;
            break;
        case 5:
            value = at<Dword>(cursor, 0);
            cursor += 4;
            break;
        }
        if (consumed_bits > 7) {
            ++cursor;
            consumed_bits = 0;
        }
        Dword destination = field & 255;
        if (!destination_width)
            at<Byte>(output, destination) = Byte(value);
        else if (destination_width == 1)
            at<std::uint16_t>(output, destination) = std::uint16_t(value);
        else if (destination_width == 3)
            at<Dword>(output, destination) = value;
    }
}
namespace {
Byte *record_base(void *resource) {
    Dword descriptors = at<Byte>(resource, 8);
    return offset(resource, (descriptors & 1 ? 10u : 12u) + descriptors * 2);
}
} // namespace

Dword __fastcall sample_record_animation(void *receiver, void *, Dword time_word, Dword output) {
    void *resource = at<void *>(receiver, 12);
    int index = at<int>(receiver, 16);
    Dword stride = at<Byte>(resource, 9);
    float time = channel_float(time_word);
    Byte *first = record_base(resource);
    auto sample = [&](int selected) {
        decode_channel_record(receiver, first + Dword(selected) * stride + 4,
                              reinterpret_cast<Byte *>(output));
        at<int>(receiver, 16) = selected;
    };
    float retained_time = at<float>(first, Dword(index) * stride);
    if (time < retained_time || std::isnan(time) || std::isnan(retained_time)) {
        for (--index; index >= 0; --index) {
            Byte *record = record_base(resource) + Dword(index) * stride;
            // Compare time with the address bits, not record->time.
            float saved_address_bits = channel_float(reinterpret_cast<Dword>(record));
            if (time >= saved_address_bits) {
                sample(index);
                return 1;
            }
        }
        sample(0);
        return 1;
    }
    int count = at<std::uint16_t>(resource, 6);
    for (; index < count; ++index) {
        Byte *record = record_base(resource) + Dword(index) * stride;
        if (time < at<float>(record, stride)) {
            sample(index);
            return 1;
        }
    }
    sample(count - 1);
    return 1;
}

Dword __fastcall find_record_animation_event(void *receiver, void *, Dword predicate, Dword limit_time,
                                            Dword time_output) {
    void *resource = at<void *>(receiver, 12);
    float limit = channel_float(limit_time);
    for (int index = 0; index < int(at<std::uint16_t>(resource, 6)); ++index) {
        Byte *record = record_base(resource) + Dword(index) * at<Byte>(resource, 9);
        float time = at<float>(record, 0);
        if (!(time <= limit)) {
            Byte decoded[80];
            decode_channel_record(receiver, record + 4, decoded);
            if (world::invoke<Byte>(reinterpret_cast<void *>(predicate), 0, decoded)) {
                *reinterpret_cast<float *>(time_output) = time;
                return 1;
            }
        }
    }
    return 0;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
