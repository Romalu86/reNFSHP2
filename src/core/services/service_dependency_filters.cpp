#include "nfs/core/services/service_dependencies.hpp"
#include <cstring>
namespace nfs::services::dependency {
FilterBucket message_filters[53]{};
FilterBucket combined_filter{}, udp_filter{}, tcp_filter{};

const char *message_name(int type) {
    static const char *const names[] = {
#include "service_dependency_message_names.inc"
    };
    return static_cast<unsigned>(type) < 52 ? names[type] : "Unknown";
}

void set_filter_name(FilterBucket *bucket, const char *name) {
    std::strcpy(bucket->name, name);
}

void reset_filter(FilterBucket *bucket) {
    bucket->accepted = bucket->rejected = bucket->bytes = bucket->recent = 0;
}

void initialize_network_filters() {
    set_filter_name(&tcp_filter, "TCP");
    set_filter_name(&udp_filter, "UDP");
    set_filter_name(&combined_filter, "Combined");
    for (int type = 0; type < 53; ++type)
        set_filter_name(message_filters + type, message_name(type));
}

void reset_network_filters() {
    reset_filter(&udp_filter);
    reset_filter(&tcp_filter);
    reset_filter(&combined_filter);
    for (auto &bucket : message_filters)
        reset_filter(&bucket);
}
} // namespace nfs::services::dependency
namespace nfs::services::pending {
void initialize_network_filters() {
    dependency::initialize_network_filters();
}
void reset_network_filters() {
    dependency::reset_network_filters();
}
} // namespace nfs::services::pending
