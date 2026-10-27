#include "frontend_multiplayer.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::pending {
void release_result_allocation(void *);
}
namespace nfs::network::session::pending {
const char *server_race_name(void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

void clear_browser_servers(BrowserView *page) {
    auto *servers = page->servers;
    page->selection = nullptr;
    if (!servers)
        return;
    auto *items = &servers->items;
    const int count = int(items->grid.rows.count);
    if (count < 1)
        return;
    const int column = page->columns[0];
    for (int row = count - 1; row >= 0; --row) {
        auto *item = invoke<family::ListItem *>(items, 9, row, column);
        if (item && item->data)
            pending::release_result_allocation(reinterpret_cast<void *>(item->data));
        invoke(items, 5, row);
    }
}

void remove_stale_browser_servers(BrowserView *page, Dword before) {
    auto *servers = page->servers;
    if (!servers)
        return;
    auto *items = &servers->items;
    const int count = int(items->grid.rows.count);
    if (count < 1)
        return;
    const int column = page->columns[0];
    for (int row = count - 1; row >= 0; --row) {
        auto *item = invoke<family::ListItem *>(items, 9, row, column);
        if (item && item->data && at<Dword>(reinterpret_cast<void *>(item->data), 700) < before) {
            item = invoke<family::ListItem *>(items, 9, row, column);
            if (item && item->data)
                pending::release_result_allocation(reinterpret_cast<void *>(item->data));
            invoke(items, 5, row);
        }
    }
}

void clear_browser_details(BrowserView *page) {
    const auto clear = [](Widget *field) {
        if (field)
            invoke(field, 7, &lifecycle::construction::unresolved::shared_string);
    };
    clear(page->track);
    clear(page->car_class);
    clear(page->dedicated);
    clear(page->bandwidth);
}

void refresh_browser_details(BrowserView *page, void *record) {
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    if (page->selection != record)
        return;
    if (page->track) {
        void *track = at<void *>(system(40), 12 + at<Dword>(record, 528) * 4);
        gui::string_reference(&text.value, at<Dword>(track, 12));
        gui::string_append_reference(&text.value, 733);
        const bool mirrored = network::lobby::server_mirrored(record) != 0;
        const bool reversed = network::lobby::server_reversed(record) != 0;
        gui::string_append_reference(&text.value, mirrored ? (reversed ? 695 : 694)
                                                           : (reversed ? 693 : 692));
        invoke(page->track, 7, &text.value);
    }
    if (page->car_class) {
        Dword reference;
        switch (at<Dword>(record, 688)) {
        case 38:
            reference = 249;
            break;
        case 39:
            reference = 250;
            break;
        case 40:
            reference = 251;
            break;
        case 41:
            reference = 252;
            break;
        case 42:
            reference = 253;
            break;
        case 43:
            reference = 40;
            break;
        default:
            reference = 38;
            break;
        }
        gui::string_reference(&text.value, reference);
        invoke(page->car_class, 7, &text.value);
    }
    if (page->dedicated) {
        gui::string_reference(&text.value, at<std::uint8_t>(record, 692) ? 374 : 1334);
        invoke(page->dedicated, 7, &text.value);
    }
    if (page->bandwidth) {
        gui::string_reference(&text.value, at<std::uint8_t>(record, 694) ? 1608 : 1610);
        if (at<std::uint8_t>(record, 694))
            gui::string_append_reference(&text.value, 1609);
        invoke(page->bandwidth, 7, &text.value);
    }
}

Dword browser_has_room(BrowserView *page) {
    if (!page->servers)
        return 0;
    const Dword count = page->servers->items.grid.rows.count;
    return (count & 4294967040u) | Dword(int(count) < page->maximum_servers);
}
namespace {
void write_server_cell(BrowserView *page, int row, int column, const gui::String *text,
                       std::uint8_t existing, Dword data, bool update_data) {
    if (!existing) {
        NetworkListItem item(text);
        item.value.data = data;
        invoke(&page->servers->items, 7, &item.value, row, column);
    } else if (auto *item = invoke<family::ListItem *>(&page->servers->items, 9, row, column)) {
        if (update_data)
            item->data = data;
        gui::string_copy(&item->text, text);
    }
}
} // namespace

void update_browser_server(BrowserView *page, void *record, std::uint8_t existing) {
    if (!page->servers || !record)
        return;
    int row;
    if (!existing) {
        row = int(page->servers->items.grid.rows.count);
        if (page->maximum_servers < row) {
            auto *first = invoke<family::ListItem *>(&page->servers->items, 9, 0, page->columns[0]);
            if (first && first->data)
                pending::release_result_allocation(reinterpret_cast<void *>(first->data));
            invoke(&page->servers->items, 5, 0);
            --row;
        }
    } else {
        row = find_browser_server_row(page, record);
        if (row < 0)
            return;
    }
    if (page->columns[0] >= 0) {
        pending::LocalFrontendString text(reinterpret_cast<const char *>(record));
        write_server_cell(page, row, page->columns[0], &text.value, existing,
                          reinterpret_cast<Dword>(record), false);
    }
    if (page->columns[1] >= 0) {
        pending::LocalFrontendString text;
        gui::string_format_ascii(&text.value, "%d/%d", at<Dword>(record, 664),
                                 at<Dword>(record, 668));
        write_server_cell(page, row, page->columns[1], &text.value, existing, 0, false);
    }
    if (page->columns[2] >= 0) {
        pending::LocalFrontendString text, format;
        gui::string_reference(&format.value, 1604);
        gui::string_format(&text.value, &format.value, at<Dword>(record, 676));
        write_server_cell(page, row, page->columns[2], &text.value, existing,
                          at<Dword>(record, 676), true);
    }
    if (page->columns[3] >= 0) {
        pending::LocalFrontendString text;
        gui::string_format_ascii(&text.value, "%s",
                                 reinterpret_cast<const char *>(bytes(record) + 24));
        write_server_cell(page, row, page->columns[3], &text.value, existing, 0, false);
    }
    if (page->columns[4] >= 0) {
        pending::LocalFrontendString text;
        gui::string_format_ascii(&text.value, "%s",
                                 network::session::pending::server_race_name(record));
        write_server_cell(page, row, page->columns[4], &text.value, existing, 0, false);
    }
    if (page->columns[5] >= 0) {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, at<std::uint8_t>(record, 693) ? 374 : 1334);
        write_server_cell(page, row, page->columns[5], &text.value, existing, 0, false);
    }
}

void remove_browser_row(BrowserView *page, int row) {
    auto *items = &page->servers->items;
    const int column = page->columns[0];
    auto *item = invoke<family::ListItem *>(items, 9, row, column);
    if (item && item->data)
        pending::release_result_allocation(reinterpret_cast<void *>(item->data));
    invoke(items, 5, row);
}

int find_browser_server_row(BrowserView *page, void *record) {
    if (!record)
        return -1;
    auto *servers = page->servers;
    if (servers) {
        auto *items = &servers->items;
        const int count = int(items->grid.rows.count);
        if (count <= 0)
            return -1;
        const int column = page->columns[0];
        for (int row = 0; row < count; ++row) {
            auto *item = invoke<family::ListItem *>(items, 9, row, column);
            if (item && item->data && item->data == reinterpret_cast<Dword>(record))
                return row;
        }
    }
    return -1;
}

void *find_browser_server(BrowserView *page, const char *address, Dword port) {
    auto *servers = page->servers;
    if (servers) {
        auto *items = &servers->items;
        const int count = int(items->grid.rows.count);
        if (count <= 0)
            return nullptr;
        const int column = page->columns[0];
        for (int row = 0; row < count; ++row) {
            auto *item = invoke<family::ListItem *>(items, 9, row, column);
            if (item && item->data) {
                void *record = reinterpret_cast<void *>(item->data);
                if (!std::strcmp(reinterpret_cast<char *>(bytes(record) + 24), address) &&
                    at<Dword>(record, 656) == port)
                    return record;
            }
        }
    }
    return nullptr;
}

int __cdecl compare_server_ping(const void *first, const void *second, Dword) {
    if (!first)
        return second != nullptr;
    if (!second)
        return -1;
    const int a = at<int>(const_cast<void *>(first), 24),
              b = at<int>(const_cast<void *>(second), 24);
    return b < a ? 1 : b <= a ? 0 : -1;
}
} // namespace nfs::application::frontend
