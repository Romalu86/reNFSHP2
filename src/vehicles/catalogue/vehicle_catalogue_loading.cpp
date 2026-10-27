#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../ai/profiles/ai_profile_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::vehicle_render {

void load_catalogue_bounds(void *entry) {
    char filename[272];
    std::sprintf(filename, "%s%s\\bounds.ini", offset(subsystem(4), 2084), offset(entry, 4));
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        const char *keys[] = {"min_x", "min_y", "min_z", "max_x", "max_y", "max_z"};
        for (unsigned i = 0; i < 6; ++i)
            ini::get_float(&reader, "boundsnormal", keys[i], &at<float>(entry, 1160 + i * 4));
        for (unsigned i = 0; i < 6; ++i)
            ini::get_float(&reader, "boundsaddon", keys[i], &at<float>(entry, 1184 + i * 4));
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
namespace {
void load_specular(ini::Reader *reader, const char *section, void *skin) {
    int addons = 0;
    ini::get_int(reader, section, "addons_enable", &addons);
    at<Byte>(skin, 16) = addons != 0;
    ini::get_float(reader, section, "specular_paint_exponent", &at<float>(skin, 32));
    ini::get_float(reader, section, "specular_glass_exponent", &at<float>(skin, 36));
    ini::get_float(reader, section, "specular_rubber_exponent", &at<float>(skin, 40));
    int sun_intensity = 0;
    ini::get_int(reader, section, "specular_sun_intensity", &sun_intensity);
    const auto format = kernel::numeric::current_format();
    at<float>(skin, 44) =
        float(kernel::numeric::Extended::integer(sun_intensity, format) * 0.0039215688593685626983642578125f);
    ini::get_int(reader, section, "specular_green", &at<int>(skin, 24));
    ini::get_int(reader, section, "specular_blue", &at<int>(skin, 28));
}
void read_vehicle_options(void *entry) {
    load_catalogue_bounds(entry);
    char filename[260];
    std::sprintf(filename, "%s%s\\vehicle.ini", offset(subsystem(4), 2084), offset(entry, 4));
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        ini::get_float(&reader, "effects", "specular_hotspot_exponent", &at<float>(entry, 1220));
        ini::get_float(&reader, "effects", "fresnel_scaler", &at<float>(entry, 1224));
        ini::get_float(&reader, "splatshadow", "scale", &at<float>(entry, 1216));
        at<Dword>(entry, 1128) = 0;
        ini::get_float(&reader, "verticalfudge", "height", &at<float>(entry, 1128));
        at<Dword>(entry, 1132) = 0;
        ini::get_float(&reader, "damage", "bouncetrunk", &at<float>(entry, 1132));
        at<Dword>(entry, 1136) = 0;
        ini::get_float(&reader, "damage", "bouncehood", &at<float>(entry, 1136));
        at<Dword>(entry, 1140) = 0;
        ini::get_int(&reader, "damage", "disabled", &at<int>(entry, 1140));
        at<float>(entry, 1144) = 2.5f;
        ini::get_float(&reader, "effects", "wheelAnimSpeed", &at<float>(entry, 1144));
        at<Dword>(entry, 1152) = 0;
        ini::get_float(&reader, "wheel_rear", "renderoffset", &at<float>(entry, 1152));
        at<Dword>(entry, 1156) = 0;
        ini::get_float(&reader, "wheel_front", "renderoffset", &at<float>(entry, 1156));
        int left_side = 0;
        ini::get_int(&reader, "driver", "leftsidedrive", &left_side);
        at<Byte>(entry, 1148) = left_side != 0;
        for (unsigned i = 0; i < 20; ++i)
            at<Dword>(entry, 64 + i * 48) = Dword(-1);
        char section[80], colour[32];
        unsigned count = 0;
        for (; int(count) < 20; ++count) {
            void *skin = offset(entry, 64 + count * 48);
            std::sprintf(section, "skin%02d", count);
            if (!ini::get_int(&reader, section, "specular_red", &at<int>(skin, 20)))
                break;
            if (ini::get_string(&reader, section, "color_id", colour, 32))
                at<Dword>(skin, 0) = runtime::ai::lifecycle::pending::parse_profile_colour(colour);
            load_specular(&reader, section, skin);
            std::sprintf(section, "paintcolor%d", count);
            char paint_name[80]{};
            if (!ini::get_string(&reader, section, "paintname", paint_name, 79)) {
                at<Dword>(skin, 8) = 4288676000u;
                at<Dword>(skin, 4) = 805;
                at<Dword>(skin, 12) = 100;
            } else {
                Dword token = 0;
                pending::resolve_text_token(paint_name, &token);
                at<Dword>(skin, 4) = token;
                int alpha = 0, red = 0, green = 0, blue = 0;
                ini::get_int(&reader, section, "painta", &alpha);
                ini::get_int(&reader, section, "paintr", &red);
                ini::get_int(&reader, section, "paintg", &green);
                ini::get_int(&reader, section, "paintb", &blue);
                at<Dword>(skin, 8) = (Dword(Byte(alpha)) << 24) | (Dword(Byte(red)) << 16) |
                                    (Dword(Byte(green)) << 8) | Byte(blue);
                at<Dword>(skin, 12) = 100;
                Dword cost; // The code does not initialize this word before the optional read.
                ini::get_int(&reader, section, "paintcost", reinterpret_cast<int *>(&cost));
                if (cost)
                    at<Dword>(skin, 12) = cost;
            }
        }
        at<Dword>(entry, 60) = count;
        at<Byte>(entry, 1024) = 0;
        at<Dword>(entry, 1028) = Dword(-1);
        if (ini::get_int(&reader, "skinhp", "specular_red", &at<int>(entry, 1048))) {
            at<Byte>(entry, 1024) = 1;
            colour[0] = 0;
            ini::get_string(&reader, "skinhp", "color_id", colour, 32);
            at<Dword>(entry, 1028) = runtime::ai::lifecycle::pending::parse_profile_colour(colour);
            load_specular(&reader, "skinhp", offset(entry, 1028));
        }
        at<Byte>(entry, 1076) = 0;
        if (ini::get_int(&reader, "skincop", "specular_red", &at<int>(entry, 1100))) {
            at<Byte>(entry, 1076) = 1;
            load_specular(&reader, "skincop", offset(entry, 1080));
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace
} // namespace nfs::game::world::vehicle_render
namespace nfs::game::world::pending {

void load_vehicle_catalog(void *owner) {
    char filename[260], section[80]{}, text[80]{};
    void *rules = subsystem(72);
    std::sprintf(filename, "%s%s", offset(subsystem(4), 2084), "Cars.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        at<Dword>(owner, 0) = 0;
        std::sprintf(section, "car%d", 0);
        while (ini::get_string(&reader, section, "name", text, 79)) {
            Dword index = at<Dword>(owner, 0);
            void *entry = offset(owner, 4 + index * 1228);
            std::strcpy(reinterpret_cast<char *>(offset(entry, 4)), text);
            at<Dword>(entry, 0) = index;
            const char *token_keys[] = {"mfrname",  "displayname", "stat0to60ID",
                                        "statTSID", "statTSKPHID", "statHPID"};
            constexpr unsigned token_fields[] = {16, 20, 44, 48, 52, 56};
            for (unsigned i = 0; i < 6; ++i) {
                Dword token = i < 2 ? 0 : Dword(-1);
                ini::get_string(&reader, section, token_keys[i], text, 79);
                resolve_text_token(text, &token);
                at<Dword>(entry, token_fields[i]) = token;
            }
            int class_index = -1;
            ini::get_int(&reader, section, "class", &class_index);
            at<Dword>(entry, 28) = Dword(class_index) + 37;
            at<Byte>(entry, 32) = 0;
            char traffic[24];
            if (ini::get_string(&reader, section, "traffic", traffic, 24))
                at<Byte>(entry, 32) = std::strstr(traffic, "true") != nullptr;
            int price = 0;
            ini::get_int(&reader, section, "price", &price);
            at<int>(entry, 36) = price;
            if (!price && !at<Byte>(entry, 32) && index < 86)
                at<Byte>(rules, 9816 + index) = 1;
            int nfs_price = 0;
            ini::get_int(&reader, section, "nfsprice", &nfs_price);
            if (nfs_price < 2001)
                nfs_price = 2000;
            at<int>(entry, 40) = nfs_price;
            at<Dword>(owner, 0) = index + 1;
            std::sprintf(section, "car%d", index + 1);
            std::sprintf(text, "%s", "");
        }
        for (int index = 0; index < at<int>(owner, 0); ++index)
            vehicle_render::read_vehicle_options(offset(owner, 4 + Dword(index) * 1228));
        for (int index = 0; index < at<int>(owner, 0); ++index) {
            void *entry = offset(owner, 4 + Dword(index) * 1228);
            if (!at<Byte>(entry, 32) && !at<Dword>(entry, 60) && at<Dword>(entry, 0) != 7)
                at<Dword>(entry, 60) = 1;
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::game::world::pending
