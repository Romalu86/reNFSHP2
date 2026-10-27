#include "nfs/physics/frame/dynamics_runtime.hpp"
#include <initializer_list>
namespace nfs::sound {
// Signed big-endian values of one, two and three bytes.
int read_bank_integer(const Byte *input, Dword length) {
    Dword value = 0, count = length;
    if (count)
        do {
            value = (value << 8) + *input++;
        } while (--count);
    if (length == 1 && static_cast<int>(value) > 127)
        value -= 256;
    else if (length == 2 && static_cast<int>(value) > 32767)
        value -= 65536;
    else if (length == 3 && static_cast<int>(value) > 8388607)
        value -= 16777216;
    return static_cast<int>(value);
}
// Long blobs deliberately retain the previous scalar field.
int next_bank_field(BankField *field) {
    while (*field->cursor == 252)
        ++field->cursor;
    Byte *start = field->cursor;
    field->tag = *start;
    if (field->tag == 255)
        return 0;
    field->cursor = start + 1;
    if (field->tag != 253 && field->tag != 254) {
        field->length = start[1];
        if (field->length == 255) {
            field->length = Dword(read_bank_integer(start + 2, 4));
            field->cursor += 4;
        }
        ++field->cursor;
        field->payload = field->cursor;
        if (field->length < 5)
            field->value = Dword(read_bank_integer(field->cursor, field->length));
        field->cursor += field->length;
    }
    return 1;
}
namespace pending {
// Low38 and extended40 property banks retain saved defaults.
int decode_layers(void **stream, void *layer) {
    BankField field;
    field.cursor = static_cast<Byte *>(*stream);
    nfs::sound::field<void *>(layer, 156) = field.cursor;
    Dword values[78], offsets[78]{};
    Dword routing[4]{};
    int more = 0;
    auto set = [&](unsigned i, Dword v) { values[i] = v; };
    set(1, 0);
    set(2, 127);
    set(3, 0);
    set(4, 127);
    set(6, 0);
    set(7, 60);
    set(8, Dword(-1));
    set(9, 1);
    set(10, 0);
    set(12, 64);
    set(13, 0);
    set(14, 127);
    for (unsigned i : {15u, 16u, 17u, 18u, 19u, 23u})
        set(i, 0);
    set(25, reinterpret_cast<Dword>(default_envelope));
    set(26, Dword(-1));
    set(28, 127);
    for (unsigned i = 29; i <= 36; ++i)
        set(i, 0);
    set(37, 1);
    set(38, 0);
    set(40, 1);
    set(42, 22050);
    set(43, 0);
    set(44, Dword(-1));
    set(45, Dword(-1));
    set(46, 0);
    set(47, 0);
    set(50, 0);
    set(58, 0);
    set(59, 0);
    set(66, 0);
    set(67, 0);
    set(68, 0);
    set(69, 0);
    set(70, 10);
    set(71, 0);
    for (unsigned i = 0; i < 4; ++i) {
        nfs::sound::field<Dword>(layer, 124 + i * 4) = 0;
        nfs::sound::field<std::uint16_t>(layer, 140 + i * 2) = 0;
    }
    nfs::sound::field<Byte>(layer, 23) = 0;
    while (next_bank_field(&field)) {
        if (field.tag < 38) {
            values[field.tag] = field.value;
            offsets[field.tag] = reinterpret_cast<Dword>(field.payload);
            if (field.tag == 20) {
                const int index = nfs::sound::field<std::int8_t>(layer, 23);
                nfs::sound::field<Dword>(layer, 52 + index * 4) =
                    reinterpret_cast<Dword>(field.payload);
                nfs::sound::field<Dword>(layer, 68 + index * 4) = field.length;
                ++nfs::sound::field<Byte>(layer, 23);
            }
        } else if (field.tag >= 128 && field.tag < 168) {
            unsigned index = 38 + field.tag - 128;
            values[index] = field.value;
            offsets[index] = reinterpret_cast<Dword>(field.payload);
            if (field.tag >= 152 && field.tag <= 155) {
                unsigned slot = field.tag - 152;
                nfs::sound::field<Dword>(layer, 124 + slot * 4) =
                    reinterpret_cast<Dword>(field.payload);
                nfs::sound::field<std::uint16_t>(layer, 140 + slot * 2) =
                    std::uint16_t(field.length);
            }
        } else if (field.tag == 254) {
            more = 1;
            break;
        }
    }
    struct Assignment {
        unsigned destination, index, width;
    };
    const Assignment assignments[] = {
        {8, 1, 1},     {9, 2, 1},     {11, 4, 1},    {12, 6, 1},    {14, 8, 1},    {15, 9, 1},
        {10, 3, 1},    {17, 12, 1},   {18, 13, 1},   {13, 7, 1},    {21, 15, 1},   {4, 16, 2},
        {16, 10, 1},   {20, 14, 1},   {6, 17, 2},    {22, 19, 1},   {120, 26, 4}, {26, 28, 1},
        {27, 30, 1},   {28, 31, 1},   {30, 35, 1},   {84, 34, 2}, {29, 33, 1},   {31, 37, 1},
        {2, 38, 1},    {19, 70, 1},   {0, 36, 2},    {86, 42, 2}, {3, 40, 1},    {88, 43, 4},
        {92, 44, 4}, {96, 45, 4}, {24, 50, 2}, {104, 46, 4}, {108, 47, 4}, {112, 58, 4},
        {116, 59, 4}};
    for (const auto &a : assignments) {
        if (a.width == 1)
            nfs::sound::field<Byte>(layer, a.destination) = Byte(values[a.index]);
        else if (a.width == 2)
            nfs::sound::field<std::uint16_t>(layer, a.destination) = std::uint16_t(values[a.index]);
        else
            nfs::sound::field<Dword>(layer, a.destination) = values[a.index];
    }
    for (const auto &a : {Assignment{32, 18, 4}, Assignment{36, 23, 4}, Assignment{40, 25, 4},
                          Assignment{44, 29, 4}, Assignment{48, 32, 4}})
        nfs::sound::field<Dword>(layer, a.destination) = values[a.index] + offsets[a.index];
    nfs::sound::field<Dword>(layer, 100) = offsets[38 + 10];
    for (unsigned i = 0; i < 4; ++i)
        routing[i] = values[66 + i];
    for (int i = 0; i < nfs::sound::field<std::int8_t>(layer, 3); ++i) {
        int channels = nfs::sound::field<std::int8_t>(layer, 3);
        if (Byte(output_channels) == 2 && channels == 2)
            routing[i] = 0;
        nfs::sound::field<std::uint16_t>(layer, 148 + i * 2) = std::uint16_t(
            nfs::sound::field<std::int16_t>(effect_groups, 36 + (i + channels * 4) * 2) +
            routing[i]);
    }
    const Dword flags = values[71];
    if (flags && !nfs::sound::field<std::uint16_t>(layer, 24)) {
        if (flags & 1)
            nfs::sound::field<std::uint16_t>(layer, 24) = 1;
        if (flags & 2)
            nfs::sound::field<Byte>(layer, 24) |= 2;
        if (Byte(flags) & 128)
            nfs::sound::field<Byte>(layer, 24) |= 32;
        if (flags & 64)
            nfs::sound::field<Byte>(layer, 24) |= 64;
    }
    *stream = field.cursor;
    return more;
}
} // namespace pending
} // namespace nfs::sound
