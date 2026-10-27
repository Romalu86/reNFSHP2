#include "gui_widget_sequence.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

void clear_image_sequence(ImageSequence *sequence) {
    Object *engine = engine_object;
    const Dword texture = sequence->texture;
    if (!texture || engine) {
        if (texture)
            invoke(engine, 34, texture);
        if (engine && invoke<Dword>(engine, 39, texture, sequence->image.image))
            clear_image_resource(&sequence->image);
    }
    sequence->texture = 0;
    sequence->frame = -1;
    sequence->frames = 0;
}

ImageSequence *copy_image_sequence(ImageSequence *sequence, const ImageSequence *source) {
    if (sequence == source || sequence->texture == source->texture)
        return sequence;
    clear_image_sequence(sequence);
    Object *engine = engine_object;
    if (engine && source->texture) {
        const Dword texture = invoke<Dword>(engine, 32, source->texture);
        sequence->texture = texture;
        if (texture) {
            const int frame = source->frame;
            sequence->frame = frame;
            sequence->frames = source->frames;
            engine = engine_object;
            clear_image_resource(&sequence->image);
            engine = engine_object;
            if (engine) {
                sequence->image.image = invoke<Dword>(engine, 36, texture, frame);
                if (sequence->image.image)
                    invoke(engine, 29, sequence->image.image, &sequence->image.bounds,
                           &sequence->image.origin);
            }
        }
    }
    return sequence;
}

ImageSequence *copy_construct_image_sequence(ImageSequence *sequence, const ImageSequence *source) {
    construct_image_sequence(sequence);
    try {
        copy_image_sequence(sequence, source);
    } catch (...) {
        destroy_image_resource(&sequence->image);
        sequence->methods = reinterpret_cast<const Method *>(gui::base_methods);
        throw;
    }
    return sequence;
}

Dword select_sequence_frame(ImageSequence *sequence, int frame) {
    Object *engine = engine_object;
    Dword texture = sequence->texture;
    if (!texture || frame < 0 || int(sequence->frames) <= frame || !engine)
        return 0;
    if (!sequence->image.image || !invoke<Dword>(engine, 39, texture, sequence->image.image)) {
        clear_image_resource(&sequence->image);
        Object *current = engine_object;
        if (!current)
            return 0;
        sequence->image.image = invoke<Dword>(current, 36, texture, frame);
        if (!sequence->image.image ||
            !invoke<Dword>(current, 29, sequence->image.image, &sequence->image.bounds,
                          &sequence->image.origin))
            return 0;
    } else if (sequence->frame == frame)
        return 1;
    const Dword image = sequence->image.image;
    if (invoke<Dword>(engine, 37, sequence->texture, image, frame))
        return 0;
    invoke(engine, 29, image, &sequence->image.bounds, &sequence->image.origin);
    sequence->frame = frame;
    return 1;
}

Dword select_named_sequence_frame(ImageSequence *sequence, const String *name) {
    if (sequence->texture && engine_object)
        return select_sequence_frame(sequence,
                                     invoke<int>(engine_object, 38, sequence->texture, name));
    return 0;
}

String *image_sequence_name(ImageSequence *sequence, String *out) {
    if (sequence->texture && engine_object)
        return invoke<String *>(engine_object, 40, out, sequence->texture);
    return string_construct_copy(out, &lifecycle::construction::unresolved::shared_string);
}

long double sequence_frame_rate(ImageSequence *sequence, Dword frame) {
    if (engine_object && sequence->texture)
        return invoke<long double>(engine_object, 41, sequence->texture, frame);
    return 0;
}

Dword create_image_sequence(ImageSequence *sequence, Dword first, Dword second, int frame,
                           Dword options) {
    clear_image_sequence(sequence);
    Object *engine = engine_object;
    if (!engine)
        return 13;
    sequence->texture = invoke<Dword>(engine, 33, first, second, options);
    if (!sequence->texture)
        return 0;
    if (!invoke<Dword>(engine, 35, sequence->texture, &sequence->frames))
        return 4;
    const Dword texture = sequence->texture;
    clear_image_resource(&sequence->image);
    engine = engine_object;
    if (!engine)
        return 13;
    sequence->image.image = invoke<Dword>(engine, 36, texture, frame);
    return sequence->image.image && invoke<Dword>(engine, 29, sequence->image.image,
                                                 &sequence->image.bounds, &sequence->image.origin)
               ? 0
               : 4;
}

Dword load_image_sequence(ImageSequence *sequence, const String *name, int frame) {
    clear_image_sequence(sequence);
    Object *engine = engine_object;
    if (!engine)
        return 13;
    sequence->texture = invoke<Dword>(engine, 31, name);
    if (!sequence->texture)
        return 0;
    if (!invoke<Dword>(engine, 35, sequence->texture, &sequence->frames))
        return 4;
    const Dword texture = sequence->texture;
    clear_image_resource(&sequence->image);
    engine = engine_object;
    if (!engine)
        return 13;
    sequence->image.image = invoke<Dword>(engine, 36, texture, frame);
    if (sequence->image.image &&
        invoke<Dword>(engine, 29, sequence->image.image, &sequence->image.bounds,
                     &sequence->image.origin) &&
        select_sequence_frame(sequence, frame))
        return 0;
    return 4;
}

Dword read_image_sequence(void *configuration, const char *key, ImageSequence *sequence) {
    const String *value = invoke<const String *>(configuration, 5, key);
    if (value->length) {
        load_image_sequence(sequence, value, 0);
        return 1;
    }
    clear_image_sequence(sequence);
    return 0;
}

Dword write_image_sequence(void *configuration, const char *key, ImageSequence *sequence) {
    String name;
    image_sequence_name(sequence, &name);
    try {
        const char *text = string_ascii(&name);
        String value;
        string_construct_ascii(&value, text);
        Dword result;
        try {
            result =
                invoke<Dword>(configuration, 7, key, &value, "ANIM", Dword(0), Dword(text == nullptr));
        } catch (...) {
            string_destruct(&value);
            throw;
        }
        string_destruct(&value);
        string_destruct(&name);
        return result;
    } catch (...) {
        string_destruct(&name);
        throw;
    }
}
} // namespace nfs::gui::widgets::family
