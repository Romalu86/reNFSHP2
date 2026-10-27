#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "gui_ime.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::gui::widgets::ime {
using game::runtime::invoke;
namespace allocations = lifecycle::construction::network::entries;

static CandidateList *construct_candidates(CandidateList *list, void *parent) {
    pending::construct_list_box(list, parent);
    list->index = 0;
    list->window = nullptr;
    list->codepage = 0;
    list->wide = 0;
    list->state = 0;
    list->capacity = 0;
    list->buffer = nullptr;
    list->base.base.base.methods = candidate_methods.data();
    list->base.items.methods = candidate_item_methods.data();
    return list;
}

static void destroy_candidates(CandidateList *list) {
    list->base.base.base.methods = candidate_methods.data();
    list->base.items.methods = candidate_item_methods.data();
    if (list->buffer)
        allocations::release_payload(list->buffer);
    family::list_box_destroy(reinterpret_cast<Widget *>(list));
}

static CandidateList *__fastcall delete_candidates(CandidateList *list, void *, Dword flags) {
    destroy_candidates(list);
    if (flags & 1)
        release(list);
    return list;
}

static CandidateList *__fastcall delete_candidate_items(family::ListItems *items, void *,
                                                        Dword flags) {
    return delete_candidates(
        reinterpret_cast<CandidateList *>(reinterpret_cast<std::uint8_t *>(items) - 264), nullptr,
        flags);
}

static const char *__fastcall candidate_name(CandidateList *, void *) {
    return "GIMECandidateList";
}

const char *__cdecl candidate_class_key() {
    return "GIMECandidateList";
}

static Dword __fastcall candidate_is_name(CandidateList *, void *, const char *name) {
    return !std::strncmp(name, "GIMECandidateList", 18) || !std::strncmp(name, "GListBox", 9) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall candidate_is_type(CandidateList *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(candidate_class_key) ||
           type == reinterpret_cast<const void *>(family::list_box_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

const std::array<Method, 58> candidate_methods = {
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::delete_candidates), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::candidate_name), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::candidate_is_type), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::candidate_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_configure_geometry), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_initialize), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_character), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_pointer_idle), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_scrollbar), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_scroll_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_column), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_check_image), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_uncheck_image), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_header), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_list_selection), // 51.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_list_column_divider), // 52.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_list_row_divider), // 53.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_list_header_background), // 54.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_cell_colour), // 55.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_list_cell), // 56.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_box_item_operation_57), // 57.
};

const std::array<Method, 14> candidate_item_methods = {
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::delete_candidate_items), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::clear_list_box_items), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::clear_list_cell), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::append_list_row), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::insert_list_row), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::erase_list_box_row), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::reorder_list_rows), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::copy_list_cell), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::append_list_cell), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_cell), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::select_list_box_item), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::select_list_box_range), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::set_list_box_selection), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::clear_list_box_selection), // 13.
};

void configure_candidates(CandidateList *list, HWND window, Dword selected_codepage, std::uint8_t wide) {
    list->window = window;
    list->codepage = selected_codepage;
    list->wide = wide;
}

void refresh_candidate_buffer(CandidateList *list) {
    HIMC current = ImmGetContext(input_window());
    if (!current)
        return;
    const auto initial_wide = list->wide;
    const Dword index = list->index;
    const DWORD bytes = initial_wide ? ImmGetCandidateListW(current, index, nullptr, 0)
                                    : ImmGetCandidateListA(current, index, nullptr, 0);
    if (!list->buffer || bytes > list->capacity) {
        if (list->buffer)
            allocations::release_payload(list->buffer);
        list->buffer = static_cast<CANDIDATELIST *>(allocations::allocate_payload(int(bytes)));
        list->capacity = bytes;
    }
    if (list->buffer) {
        if (list->wide)
            ImmGetCandidateListW(current, index, list->buffer, list->capacity);
        else
            ImmGetCandidateListA(current, index, list->buffer, list->capacity);
    }
    ImmReleaseContext(list->window, current);
}

void refresh_candidates(Edit *edit) {
    CandidateList *list = nullptr;
    if (edit->candidate_id) {
        auto *candidate = events::find_widget(context_state, edit->candidate_id);
        if (candidate && invoke<Dword>(candidate, 2, candidate_class_key))
            list = static_cast<CandidateList *>(candidate);
    }
    if (list) {
        configure_candidates(list, input_window(), codepage, unicode_composition);
        refresh_candidate_buffer(list);
    }
}

static void *create_candidates(void *parent) {
    auto *list =
        static_cast<CandidateList *>(allocate_flags(sizeof(CandidateList), 0, "GIMECandidateList"));
    if (!list)
        return nullptr;
    try {
        return construct_candidates(list, parent);
    } catch (...) {
        release_flags(list, 0, "GIMECandidateList");
        throw;
    }
}
static ClassRecord candidate_class;

static void unregister_candidates() {
    unregister_class(&candidate_class);
}

static void register_candidates() {
    register_class(&candidate_class, "GIMECandidateList", create_candidates, "GListBox");
    std::atexit(unregister_candidates);
}
} // namespace nfs::gui::widgets::ime
NFS_KERNEL_INITIALIZER(startup_register_candidates, ".CRT$XCU_NFS_00690094",
                       nfs::gui::widgets::ime::register_candidates);
