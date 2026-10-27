#include "player_profile.hpp"
#include <cstring>
namespace nfs::profiles {
void copy_profile_dwords(void *destination, const void *source, Dword count) {
    auto *target = bytes(destination);
    const auto *input = bytes(source);
    while (count--) {
        Dword value;
        std::memcpy(&value, input, sizeof(value));
        std::memcpy(target, &value, sizeof(value));
        input += sizeof(value);
        target += sizeof(value);
    }
}

namespace {
// MOVSD ranges proceed forwards, including unaligned fields and self-assignment.
void copy_dwords(void *destination, const void *source, unsigned offset, unsigned size) {
    copy_profile_dwords(bytes(destination) + offset, bytes(source) + offset,
                        size / sizeof(Dword));
}
} // namespace

void copy_preferences(void *destination, const void *source) {
    copy_dwords(destination, source, 0, 24);
    at<std::uint16_t>(destination, 24) = at<std::uint16_t>(source, 24);
    copy_dwords(destination, source, 26, 24);
    const Dword profile_flags = at<Dword>(source, 52);
    at<std::uint16_t>(destination, 50) = at<std::uint16_t>(source, 50);
    at<Dword>(destination, 52) = profile_flags;
    for (unsigned index = 0; index < 26; ++index) {
        auto *target = bytes(destination) + 56 + index * 4;
        const auto *binding = bytes(source) + 56 + index * 4;
        if (target == binding)
            continue;
        const Byte type = at<volatile Byte>(binding, 0);
        if (!type)
            at<volatile Dword>(target, 0) = 0;
        else if (type >= 1 && type <= 4) {
            if (type == 4) {
                const auto control = at<volatile std::uint16_t>(binding, 2);
                const Byte device = at<volatile Byte>(binding, 1);
                at<volatile Byte>(target, 0) = 4;
                at<volatile Byte>(target, 1) = device;
                at<volatile std::uint16_t>(target, 2) = control;
            } else {
                const Dword value = at<volatile Dword>(binding, 0);
                if (type == 1) {
                    const Byte qualifiers = at<volatile Byte>(binding, 3) & 15;
                    const Byte device = at<volatile Byte>(binding, 1);
                    const Byte control = at<volatile Byte>(binding, 2);
                    at<volatile Byte>(target, 0) = 1;
                    at<volatile Byte>(target, 2) = control;
                    at<volatile Byte>(target, 1) = device;
                    const Dword partial = at<volatile Dword>(target, 0);
                    at<volatile Dword>(target, 0) = (partial & 16777215) ^
                                                  (Dword(qualifiers) << 24) ^
                                                  (value & 4026531840u);
                } else if (type == 2) {
                    at<volatile Byte>(target, 0) = 2;
                    const Dword partial = at<volatile Dword>(target, 0);
                    at<volatile Dword>(target, 0) = (partial & 255) ^
                                                  (((value >> 8) & 1048575) << 8) ^
                                                  (value & 4026531840u);
                } else {
                    const Byte qualifiers = at<volatile Byte>(binding, 3) & 15;
                    const Dword packed = at<volatile std::uint16_t>(binding, 1);
                    at<volatile Byte>(target, 0) = 3;
                    const Dword partial = at<volatile Dword>(target, 0);
                    const Dword low = ((((packed << 8) ^ partial) & 16776960) ^ partial) &
                                      16777215;
                    at<volatile Dword>(target, 0) = low ^ (Dword(qualifiers) << 24) ^
                                                  (value & 4026531840u);
                }
            }
        }
    }
    copy_dwords(destination, source, 160, 7252);
    at<std::uint16_t>(destination, 7412) = at<std::uint16_t>(source, 7412);
    copy_dwords(destination, source, 7414, 96);
    copy_dwords(destination, source, 7512, 32);
    at<Byte>(destination, 7544) = at<Byte>(source, 7544);
    at<Byte>(destination, 7545) = at<Byte>(source, 7545);
    copy_dwords(destination, source, 7548, preferences_size - 7548);
}
} // namespace nfs::profiles
