#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::runtime::ai::lifecycle {

Dword *construct_entity_list(Dword *list) {
    list[0] = list[2501] = list[2502] = 0;
    return list;
}

void destroy_entity_list(Dword *list) {
    list[0] = 0;
}

void clear_entity_list(Dword *list) {
    for (int index = 0; index < static_cast<int>(list[0]); ++index) {
        auto *entity = reinterpret_cast<Dword *>(list[index + 1]);
        if (entity)
            invoke(entity, 5, Dword{1});
        list[index + 1] = 0;
    }
    list[0] = list[2501] = list[2502] = 0;
}

void reset_pursuit_schedule(Dword *schedule) {
    for (unsigned index = 0; index < 8; ++index)
        schedule[index + 2] = 0;
    schedule[1] = schedule[10] = 0;
    auto *rules = system(72);
    const int difficulty = at<int>(rules, 200);
    const float base = difficulty == 2 ? 10.0f : difficulty == 3 ? 5.0f : 20.0f;
    at<float>(schedule, 44) = base;
    const Dword player_count = at<Dword>(rules, 176);
    const Dword racer_count = at<Dword>(rules, 180);
    const Dword cop_count = at<Dword>(rules, 172);
    const Dword total = player_count + racer_count + cop_count;
    at<float>(schedule, 44) =
        float(kernel::numeric::Extended::integer(static_cast<std::int32_t>(total),
                                                kernel::numeric::current_format()) +
              at<float>(schedule, 44));
}

void clear_pursuit_schedule(Dword *schedule) {
    for (unsigned index = 0; index < 8; ++index)
        schedule[index + 2] = 0;
    schedule[1] = schedule[10] = 0;
}

void begin_path_level() {
}  

void end_path_level() {
}  

int selectable_car_count(Dword *catalog) {
    int count = 0;
    for (int index = 0; index < static_cast<int>(*catalog); ++index)
        if (at<Byte>(catalog, 36 + index * 1228))
            ++count;
    return count;
}

int selectable_car(Dword *catalog, int ordinal) {
    int count = 0;
    for (int index = 0; index < static_cast<int>(*catalog); ++index)
        if (at<Byte>(catalog, 36 + index * 1228) && count++ == ordinal)
            return at<int>(catalog, 4 + index * 1228);
    return 25;
}

Dword car_variant_count(Dword *catalog, int kind) {
    return at<Dword>(catalog, 64 + kind * 1228);
}

void reset_chase_contact(void *contact) {
    at<Byte>(contact, 0) = 0;
    at<Dword>(contact, 4) = 4294967295u;
    for (unsigned offset = 8; offset <= 24; offset += 4)
        at<Dword>(contact, offset) = 0;
}
} // namespace nfs::game::runtime::ai::lifecycle
