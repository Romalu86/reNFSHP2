#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include <algorithm>

namespace nfs::game::runtime::ai::lifecycle {

void *reset_path_reference(void *reference) {
    at<Dword>(reference, 0) = 0;
    at<Dword>(reference, 4) = 4294967295u;
    at<Dword>(reference, 8) = 0;
    at<std::uint16_t>(reference, 44) = 0;
    return reference;
}

void *copy_path_reference(void *destination, const void *source) {
    if (destination == source)
        return destination;
    for (unsigned index = 0; index < 3; ++index)
        at<Dword>(destination, index * 4) = at<Dword>(source, index * 4);
    at<std::uint16_t>(destination, 44) =
        std::min<std::uint16_t>(at<std::uint16_t>(source, 44), 8);
    // The loop copies forward and reloads the count after each write.
    for (unsigned index = 0; index < at<std::uint16_t>(destination, 44); ++index)
        at<Dword>(destination, 12 + index * 4) = at<Dword>(source, 12 + index * 4);
    return destination;
}

int path_length(void *path, int maximum) {
    int length;
    if (at<int>(path, 20) < 1) {
        length = at<Dword>(path, 32) ? at<int>(at<void *>(path, 36), 36) : 0;
        length = static_cast<int>(Dword(length) + at<Dword>(path, 16));
    } else {
        length = at<Dword>(path, 20) ? at<int>(at<void *>(path, 24), 36) : 0;
        if (!at<Dword>(path, 32))
            length = static_cast<int>(Dword(length) - at<Dword>(path, 16));
        else
            length = static_cast<int>(at<Dword>(at<void *>(path, 36), 36) -
                                      at<Dword>(path, 16) + Dword(length));
    }
    if (maximum && maximum < length)
        length = maximum;
    return length;
}

void locate_path_sample(void *path, int distance, void **selected, int *sample) {
    const int direction = at<int>(path, 20);
    void *cursor = bytes(path) + 16;
    *sample = static_cast<int>(at<Dword>(path, 16) +
                               (direction > 0 ? Dword(distance) : Dword(0) - Dword(distance)));
    Dword endpoint;
    void *neighbor;
    int displacement;
    bool crosses = false;
    if (*sample < 0) {
        endpoint = at<Dword>(at<void *>(path, 24), 20);
        neighbor = bytes(path) + (direction > 0 ? 4 : 28);
        displacement = static_cast<int>(Dword(0) - Dword(*sample));
        crosses = true;
    } else {
        const int length = at<Dword>(path, 20) ? at<int>(at<void *>(path, 24), 36) : 0;
        if (*sample >= length) {
            endpoint = at<Dword>(at<void *>(path, 24), 24);
            neighbor = bytes(path) + (direction > 0 ? 28 : 4);
            displacement = static_cast<int>(Dword(*sample) - Dword(length) + 1);
            crosses = true;
        }
    }
    if (crosses && neighbor && at<void *>(neighbor, 8)) {
        cursor = neighbor;
        void *section = at<void *>(neighbor, 8);
        if (at<Dword>(section, 20) == endpoint)
            *sample = displacement;
        else if (!at<Dword>(neighbor, 4))
            *sample = static_cast<int>(-1u - Dword(displacement));
        else
            *sample = static_cast<int>(at<Dword>(section, 36) - Dword(displacement) - 1);
    }
    if (*sample < 0)
        *sample = 0;
    else {
        const int length = at<Dword>(cursor, 4) ? at<int>(at<void *>(cursor, 8), 36) : 0;
        if (*sample >= length)
            *sample = at<Dword>(cursor, 4) ? at<int>(at<void *>(cursor, 8), 36) - 1 : -1;
    }
    *selected = cursor;
}

void *path_section_at(void *path, int distance) {
    void *selected;
    int sample;
    locate_path_sample(path, distance, &selected, &sample);
    return at<void *>(selected, 8);
}

void sample_path(void *path, int distance, float *pose) {
    void *selected;
    int sample;
    locate_path_sample(path, distance, &selected, &sample);
    pending::sample_section_pose(at<void *>(selected, 8), sample, pose);
}

void *path_current_section(void *cursor) {
    return at<void *>(cursor, 8);
}

void clear_path(void *path) {
    for (unsigned offset = 4; offset <= 36; offset += 4)
        at<Dword>(path, offset) = 0;
}

void choose_least_used_section(void *reference) {
    void *selected = nullptr;
    int least = 0;
    for (unsigned index = 0; index < at<std::uint16_t>(reference, 44); ++index) {
        void *candidate = at<void *>(reference, 12 + index * 4);
        if (!selected || at<int>(candidate, 32) < least) {
            selected = candidate;
            least = at<int>(candidate, 32);
        }
    }
    if (selected)
        at<void *>(reference, 8) = selected;
}

void *reverse_path_reference(void *reference) {
    void *section = at<void *>(reference, 8);
    if (!section) {
        at<std::uint16_t>(reference, 44) = 0;
        return bytes(reference) + 12;
    }
    if (at<Dword>(reference, 4) == at<Dword>(section, 20))
        at<Dword>(reference, 4) = at<Dword>(section, 24);
    else if (at<Dword>(reference, 4) == at<Dword>(section, 24))
        at<Dword>(reference, 4) = at<Dword>(section, 20);
    return pending::enumerate_path_sections(reference, 1);
}

void reverse_path(void *path) {
    reverse_path_reference(bytes(path) + 40);
    choose_least_used_section(bytes(path) + 40);
    for (unsigned offset = 4; offset <= 12; offset += 4)
        std::swap(at<Dword>(path, offset), at<Dword>(path, offset + 24));
    for (unsigned offset : {8u, 20u, 32u})
        at<Dword>(path, offset) = 0u - at<Dword>(path, offset);
}

void bind_path(void *path, const void *reference, Dword sample, Dword direction, void *entity) {
    copy_path_reference(bytes(path) + 40, reference);
    const Dword section = at<Dword>(reference, 8);
    if (section == at<Dword>(path, 36)) {
        for (unsigned offset = 4; offset <= 12; offset += 4)
            at<Dword>(path, offset) = at<Dword>(path, offset + 12);
    } else
        for (unsigned offset = 4; offset <= 12; offset += 4)
            at<Dword>(path, offset) = 0;
    at<Dword>(path, 16) = sample;
    at<Dword>(path, 20) = direction;
    at<Dword>(path, 0) = 1;
    at<Dword>(path, 24) = section;
    pending::update_path_branch(path, entity);
}
} // namespace nfs::game::runtime::ai::lifecycle
