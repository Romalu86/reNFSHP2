#include "frontend_race_results.hpp"
namespace nfs::application::pending {

Dword participant_award(const RaceParticipant *record, Dword award) {
    return record->awards[award];
}

Dword result_has_event_bonus() { return services::special_race_mode(system(72)); }

void release_result_allocation(void *allocation) {
    if (allocation) kernel::memory_free(allocation);
}
}
namespace nfs::gui::widgets::family {

ListItem *construct_text_list_item(ListItem *item, const gui::String *text) {
    gui::string_construct(&item->text);
    try {
        gui::string_construct(&item->tooltip);
        try {
            item->methods = list_item_methods;
            gui::string_copy(&item->text, text);
            item->data = item->flags = item->image = 0;
        } catch (...) { gui::string_destruct(&item->tooltip); throw; }
    } catch (...) { gui::string_destruct(&item->text); throw; }
    return item;
}
}
