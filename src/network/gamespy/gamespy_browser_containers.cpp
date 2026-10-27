#include "gamespy_browser.hpp"
namespace nfs::network::session::pending::browser {

void insert_array(void *array, const void *value, int index) {
    const int count = at<int>(array, 0);
    if (count == at<int>(array, 4)) {
        at<Dword>(array, 4) += at<Dword>(array, 12);
        at<void *>(array, 20) =
            std::realloc(at<void *>(array, 20), at<Dword>(array, 8) * at<Dword>(array, 4));
    }
    at<Dword>(array, 0) = Dword(count) + 1;
    const Dword stride = at<Dword>(array, 8);
    auto *destination = bytes(at<void *>(array, 20)) + Dword(index) * stride;
    if (index < count)
        std::memmove(destination + stride, destination, (Dword(count) - Dword(index)) * stride);
    std::memcpy(destination, value, stride);
}

void remove_array_without_destroy(void *array, int index) {
    const int count = int(at<Dword>(array, 0) - 1);
    if (index < count) {
        const Dword stride = at<Dword>(array, 8);
        auto *destination = bytes(at<void *>(array, 20)) + Dword(index) * stride;
        std::memmove(destination, destination + stride, (Dword(count) - Dword(index)) * stride);
    }
    at<int>(array, 0) = count;
}

void replace_array(void *array, const void *value, int index) {
    auto *destination = bytes(at<void *>(array, 20)) + at<Dword>(array, 8) * Dword(index);
    if (auto destroy = at<Destroy>(array, 16))
        destroy(destination);
    std::memcpy(destination, value, at<Dword>(array, 8));
}

int search_array(void *array, const void *key, Compare compare, int start, Dword sorted) {
    if (!array || !at<Dword>(array, 0))
        return -1;
    const int stride = at<int>(array, 8);
    auto *storage = bytes(at<void *>(array, 20));
    auto *first = storage + Dword(start) * Dword(stride);
    const int count = int(at<Dword>(array, 0) - Dword(start));
    if (!sorted) {
        for (int offset = 0; offset < count; ++offset)
            if (!compare(key, first + Dword(offset) * Dword(stride))) {
                auto *found = first + Dword(offset) * Dword(stride);
                if (found)
                    return static_cast<int>(found - storage) / stride;
                return -1;
            }
        return -1;
    }
    int low = 0, high = int(Dword(count) - 1);
    bool matched = false;
    while (low <= high) {
        const int middle = static_cast<int>(Dword(low) + Dword(high)) >> 1;
        const int result = compare(first + Dword(middle) * Dword(stride), key);
        if (!result)
            matched = true;
        if (result >= 0)
            high = int(Dword(middle) - 1);
        else
            low = int(Dword(middle) + 1);
    }
    auto *found = first + Dword(low) * Dword(stride);
    return found && matched ? static_cast<int>(found - storage) / stride : -1;
}

HashTable *create_hash(int stride, int bucket_count, int capacity, Hash hash, Compare compare,
                       Destroy destroy) {
    auto *table = static_cast<HashTable *>(std::malloc(20));
    table->buckets = static_cast<void **>(std::malloc(Dword(bucket_count) * 4));
    for (int index = 0; index < bucket_count; ++index)
        table->buckets[index] = http::allocate_dynamic_array(stride, capacity, destroy);
    table->bucket_count = bucket_count;
    table->hash = hash;
    table->compare = compare;
    table->destroy = destroy;
    return table;
}

void free_hash(HashTable *table) {
    for (int index = 0; index < table->bucket_count; ++index)
        http::free_dynamic_array(table->buckets[index]);
    std::free(table->buckets);
    std::free(table);
}

void enter_hash(HashTable *table, const void *entry) {
    void *bucket = table->buckets[table->hash(entry, Dword(table->bucket_count))];
    const int index = search_array(bucket, entry, table->compare, 0, 0);
    if (index == -1)
        http::append_dynamic_array(bucket, entry);
    else
        replace_array(bucket, entry, index);
}

void *find_hash(HashTable *table, const void *entry) {
    void *bucket = table->buckets[table->hash(entry, Dword(table->bucket_count))];
    const int index = search_array(bucket, entry, table->compare, 0, 0);
    return index == -1 ? nullptr : http::dynamic_array_item(bucket, index);
}
} // namespace nfs::network::session::pending::browser
