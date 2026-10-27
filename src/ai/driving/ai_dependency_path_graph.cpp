#include "nfs/ai/lifecycle/ai_dependencies.hpp"
namespace nfs::game::runtime::ai::dependencies {

void *enumerate_path_sections(void *reference, Dword include_previous) {
    const int endpoint = at<int>(reference, 4);
    const Dword previous = at<Dword>(reference, 8);
    at<std::uint16_t>(reference, 44) = 0;
    if (endpoint >= 0 && previous) {
        void *section = reinterpret_cast<void *>(previous);
        Dword neighbor = include_previous;
        if (Dword(endpoint) == at<Dword>(section, 20))
            neighbor = at<Dword>(section, 24);
        else if (Dword(endpoint) == at<Dword>(section, 24))
            neighbor = at<Dword>(section, 20);
        void *group = at<void *>(reference, 0);
        void *junction = bytes(group) + neighbor * 52;
        if (at<std::uint16_t>(junction, 48) == 1) {
            const Dword candidate = at<std::uint16_t>(junction, 48) ? at<Dword>(junction, 16) : 0;
            auto &count = at<std::uint16_t>(reference, 44);
            if (count < 8) {
                at<Dword>(reference, 12 + count * 4) = candidate;
                ++count;
            }
            at<Dword>(reference, 4) = neighbor;
            return bytes(reference) + 12;
        }
        at<Dword>(reference, 4) = neighbor;
        at<Dword>(reference, 8) = 0;
        for (Dword index = 0;
             index < at<std::uint16_t>(
                         bytes(at<void *>(reference, 0)) + at<Dword>(reference, 4) * 52, 48);
             ++index) {
            junction = bytes(at<void *>(reference, 0)) + at<Dword>(reference, 4) * 52;
            const Dword candidate = std::uint16_t(index) < at<std::uint16_t>(junction, 48)
                                       ? at<Dword>(junction, 16 + std::uint16_t(index) * 4)
                                       : 0;
            if (static_cast<Byte>(include_previous) || candidate != previous) {
                auto &count = at<std::uint16_t>(reference, 44);
                if (count < 8) {
                    at<Dword>(reference, 12 + count * 4) = candidate;
                    ++count;
                }
            }
        }
    }
    return bytes(reference) + 12;
}
} // namespace nfs::game::runtime::ai::dependencies
