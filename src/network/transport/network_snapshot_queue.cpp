#include "nfs/network/vehicles/network_snapshots.hpp"
namespace nfs::network::snapshots {
using namespace packets;
namespace {
void copy_snapshot(void *out, const void *source) {
    for (unsigned index = 0; index < 20; ++index)
        at<Dword>(out, index * 4) = at<Dword>(const_cast<void *>(source), index * 4);
}
} // namespace

void *allocate(void *owner) {
    for (unsigned index = 0; index < 48; ++index)
        if (!at<Dword>(owner, 220 + index * 92))
            return bytes(owner) + 136 + index * 92;
    return nullptr;
}

void insert(void *owner, const void *source, void *before, Dword kind) {
    void *node = allocate(owner);
    if (!before || !node)
        return;
    for (unsigned index = 0; index < 48; ++index)
        if (at<Dword>(owner, 220 + index * 92) &&
            at<void *>(owner, 216 + index * 92) == before) {
            at<void *>(owner, 216 + index * 92) = node;
            break;
        }
    at<void *>(node, 80) = before;
    if (at<void *>(owner, 4556) == before)
        at<void *>(owner, 4556) = node;
    copy_snapshot(node, source);
    at<Dword>(node, 84) = 1;
    at<Dword>(node, 88) = kind;
}

void append(void *owner, const void *source, Dword kind) {
    void *node = allocate(owner);
    if (!node)
        return;
    if (void *tail = at<void *>(owner, 4560))
        at<void *>(tail, 80) = node;
    else
        at<void *>(owner, 4556) = node;
    at<void *>(owner, 4560) = node;
    copy_snapshot(node, source);
    at<void *>(node, 80) = nullptr;
    at<Dword>(node, 84) = 1;
    at<Dword>(node, 88) = kind;
}

Dword pop(void *owner, void *destination) {
    Dword kind = 0;
    if (void *head = at<void *>(owner, 4556)) {
        copy_snapshot(destination, head);
        kind = at<Dword>(head, 88);
        at<Dword>(head, 84) = 0;
        at<void *>(owner, 4556) = at<void *>(head, 80);
    }
    if (!at<void *>(owner, 4556))
        at<void *>(owner, 4560) = nullptr;
    return kind;
}
} // namespace nfs::network::snapshots
