#include "nfs/audio/speech/speech_queue.hpp"

namespace nfs::speech {

Byte *find_event_definition(const Dword *event) {
    EventGroup *group;
    if (find_event_group(event, &group))
        return find_group_event(group->table, *reinterpret_cast<const std::uint16_t *>(event));
    return nullptr;
}


Byte *find_group_event(Byte *table, Dword key) {
    const auto count = *reinterpret_cast<const std::uint16_t *>(table + 16);
    const auto *indices = reinterpret_cast<const std::uint16_t *>(table + 24);
    for (unsigned index = 0; index < count; ++index) {
        Byte *definition = table + indices[index] * 4;
        if (*reinterpret_cast<const std::uint16_t *>(definition) == key)
            return definition;
    }
    return nullptr;
}


int find_event_group(const Dword *event, EventGroup **output) {
    auto &groups = state<EventGroup[8]>(8769664);
    const auto *identifier = reinterpret_cast<const Byte *>(event);
    for (auto &group : groups) {
        if (group.table && group.table[9] == identifier[2] && group.table[8] == identifier[3]) {
            *output = &group;
            return 1;
        }
    }
    return 0;
}


int read_event_voice(const Dword *event, int *output) {
    EventGroup *group;
    if (find_event_group(event, &group)) {
        *output = group->voice;
        return 1;
    }
    return 0;
}


int read_event_context(const Dword *event, Dword *output) {
    EventGroup *group;
    if (find_event_group(event, &group)) {
        *output = group->table[9];
        return 1;
    }
    return 0;
}


void clear_event_groups() {
    for (auto &group : state<EventGroup[8]>(8769664)) {
        group.table = nullptr;
        group.voice = 0;
    }
}


std::uint16_t read_event_threshold(const Dword *event) {
    EventGroup *group;
    if (find_event_group(event, &group))
        return *reinterpret_cast<const std::uint16_t *>(group->table + 20);
    return 0;
}
} // namespace nfs::speech
