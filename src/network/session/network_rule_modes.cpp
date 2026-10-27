#include "nfs/network/session/network_rules.hpp"
namespace nfs::network::rules {
using namespace packets;
// Menu ranges, including the 100-entry mode.
void set_mode(Dword *owner, int mode) {
    at<int>(owner, 152) = mode;
    if (at<int>(owner, 228) == -1)
        at<Dword>(owner, 228) = 2;
    if (mode == 5)
        at<Dword>(owner, 228) = 100;
    if (at<Dword>(owner, 148) == 1)
        return;
    auto range = [&](Dword id, unsigned count, int start) {
        for (unsigned i = 0; i < count; ++i)
            at<int>(owner, 12 + i * 4) = start + int(i);
        choice_values(system(96), id, count, bytes(owner) + 12);
    };
    auto one = [&](Dword id, Dword value) {
        at<Dword>(owner, 12) = value;
        choice_values(system(96), id, 1, bytes(owner) + 12);
    };
    switch (mode) {
    case 0:
        range(7, 8, 1);
        one(8, 1);
        range(14, 8, 0);
        range(11, 3, 0);
        range(12, 4, 0);
        break;
    case 1:
        if (!at<Dword>(owner, 180))
            at<Dword>(owner, 180) = 1;
        one(7, at<Dword>(owner, 180));
        choice_selected(system(96), 7, at<Dword>(owner, 180));
        one(8, 1);
        range(14, 8, 1);
        choice_selected(system(96), 14, at<Dword>(owner, 180));
        range(11, 3, 0);
        range(12, 4, 0);
        break;
    case 2:
        range(7, 8, 1);
        range(8, 7, 1);
        one(14, 8);
        range(11, 3, 0);
        range(12, 4, 0);
        break;
    case 3:
        range(7, 8, 1);
        one(8, 1);
        one(14, 0);
        range(11, 3, 0);
        range(12, 4, 0);
        break;
    case 4:
        choice_values(system(96), 7, 0, bytes(owner) + 12);
        one(8, 1);
        range(14, 9, 0);
        one(11, 1);
        range(12, 4, 0);
        at<Dword>(owner, 164) = 18000;
        at<Dword>(owner, 256) = 1;
        return;
    case 5:
        at<Dword>(owner, 228) = 100;
        range(7, 100, 1);
        one(8, 1);
        one(14, 0);
        one(11, 0);
        one(12, 0);
        break;
    }
    at<Dword>(owner, 256) = 0;
}
} // namespace nfs::network::rules
