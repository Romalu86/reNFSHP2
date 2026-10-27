#include "player_profile.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include <cstring>
#include <cstdlib>
namespace nfs::profiles {

void reset_secondary_bindings(void *owner) {
    for (unsigned index = 0; index < 32; ++index) {
        auto *entry = bytes(owner) + index * 16;
        at<Dword>(entry, 0) = Dword(-1);
        at<Dword>(entry, 4) = 0;
        at<Dword>(entry, 8) = 0;
        at<Dword>(entry, 12) = 0;
    }
}
// No extra terminator is stored beyond strncpy's63 bytes.
Byte append_controller_preference(void *owner, Dword device, const char *name) {
    const int count = at<int>(owner, 204);
    if (count >= 3)
        return 0;
    char *entry = reinterpret_cast<char *>(bytes(owner) + Dword(count) * 68);
    at<int>(owner, 204) = count + 1;
    at<Dword>(entry, 64) = device;
    std::strncpy(entry, name, 63);
    return 1;
}

void capture_controller_preferences(void *controller_list, void *destination) {
    Dword count = at<Dword>(controller_list, 0);
    auto *entry = bytes(controller_list) + 4;
    while (count--) {
        if (void *device = at<void *>(entry, 4))
            append_controller_preference(destination, at<Dword>(entry, 0),
                                         reinterpret_cast<const char *>(bytes(device) + 8));
        entry += 8;
    }
}
// Only the stores present in the constructor are made.
void construct_preferences(void *owner) {
    for (unsigned index = 0; index < 26; ++index)
        at<Dword>(owner, 56 + index * 4) = 0;
    for (unsigned bank : {8180u, 28772u})
        for (unsigned index = 0; index < 33; ++index) {
            void *entry = bytes(owner) + bank + index * 624;
            at<Dword>(entry, 0) = 0;
            reset_progress_record(bytes(entry) + 4);
        }
    void *controller_preferences = bytes(owner) + 49364;
    for (unsigned index = 0; index < 3; ++index) {
        void *entry = bytes(controller_preferences) + index * 68;
        at<Dword>(entry, 64) = Dword(-1);
        at<Byte>(entry, 63) = at<Byte>(entry, 0) = 0;
    }
    at<Dword>(owner, 49568) = 0;
    reset_controller_preferences(controller_preferences);
    reset_secondary_bindings(bytes(owner) + 49572);
    at<Dword>(owner, 50084) = 0;
    struct Span {
        unsigned offset, size;
    };
    constexpr Span spans[] = {{164, 56},   {220, 24},    {244, 448},    {692, 12},
                              {704, 12},   {716, 4128}, {4844, 48},   {4892, 344},
                              {5240, 48}, {5288, 344}, {5632, 344},  {5976, 344},
                              {6324, 48}, {6372, 344}, {6728, 56},   {6784, 448},
                              {7244, 56}, {7300, 24},  {8180, 20592}, {28772, 20592},
                              {7328, 86}, {7462, 48},  {7414, 48},   {7512, 32}};
    for (const auto span : spans)
        std::memset(bytes(owner) + span.offset, 0, span.size);
    for (unsigned offset : {5236u, 6320u, 6716u, 6720u, 6724u, 7232u, 7236u, 7240u,
                            7324u, 7556u, 7548u, 7552u})
        at<Dword>(owner, offset) = 0;
    at<Dword>(owner, 8180) = at<Dword>(owner, 28772) = 1;
    for (unsigned offset : {7397u, 7333u, 7337u, 7342u, 7349u, 7350u, 7462u, 7502u})
        at<Byte>(owner, offset) = 1;
    at<Dword>(owner, 52) = 0;
    if (launch::field<Dword>(8627896))
        at<Dword>(owner, 7240) = Dword(std::atoi(&launch::field<char>(8629444)));
    at<Dword>(owner, 7512) = at<Dword>(owner, 7536) = at<Dword>(owner, 7540) = 0;
    at<Dword>(owner, 7516) = at<Dword>(owner, 7528) = 9;
    at<Dword>(owner, 7520) = 69;
    at<Dword>(owner, 7524) = 17;
    reset_progress_record(bytes(owner) + 7560);
    at<Dword>(owner, 160) = 0;
    at<Byte>(owner, 7545) = at<Byte>(owner, 7544) = 1;
}
// The 26 element destructors have no work.
void destroy_preferences(void *) {
}

void reset_preferences(void *owner) {
    at<Dword>(owner, 8180) = at<Dword>(owner, 28772) = 1;
    at<Dword>(owner, 160) = 0;
    void *catalog = system(52);
    for (int index = 0; index < at<int>(catalog, 4); ++index)
        if (!at<Dword>(catalog, 44 + unsigned(index) * 1228) &&
            !at<Byte>(catalog, 36 + unsigned(index) * 1228) && unsigned(index) < 86)
            at<Byte>(owner, 7328 + unsigned(index)) = 1;
    for (unsigned offset : {7397u, 7333u, 7337u, 7342u, 7349u, 7350u, 7462u, 7502u})
        at<Byte>(owner, offset) = 1;
    void *tracks = system(40);
    const int count = at<int>(tracks, 4);
    for (int index = 0; index < count; ++index) {
        const void *track = at<void *>(tracks, 12 + unsigned(index) * 4);
        for (unsigned variant = 0; variant < 4; ++variant)
            if (!at<Dword>(track, 32 + variant * 4) && index < 12)
                at<Byte>(owner, 7462 + unsigned(index) * 4 + variant) = 1;
    }
    at<Dword>(owner, 52) = 0;
    if (launch::field<Dword>(8627896))
        at<Dword>(owner, 7240) = Dword(std::atoi(&launch::field<char>(8629444)));
    at<Dword>(owner, 7512) = at<Dword>(owner, 7536) = at<Dword>(owner, 7540) = 0;
    at<Dword>(owner, 7516) = at<Dword>(owner, 7528) = 9;
    at<Dword>(owner, 7520) = 69;
    at<Dword>(owner, 7524) = 17;
    at<Byte>(owner, 7545) = at<Byte>(owner, 7544) = 1;
    reset_controller_preferences(bytes(owner) + 49364);
    reset_secondary_bindings(bytes(owner) + 49572);
    capture_controller_preferences(controllers::get_bindings(), bytes(owner) + 49364);
}
} // namespace nfs::profiles
