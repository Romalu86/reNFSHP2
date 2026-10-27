#include "nfs/resources/archives/big_directory.hpp"
#include <cstring>

namespace nfs {
std::uint32_t big_read_field(const std::uint8_t* data, int width) noexcept {

    const volatile std::uint8_t* bytes = data;
    switch(width) {
    case 1:return bytes[0];
    case 2: {
        const std::uint32_t first = bytes[0];
        const std::uint32_t second = bytes[1];
        return (first << 8) | second;
    }
    case 3:case 4: {
        // The three leading bytes are read in this order by both cases.
        const std::uint32_t first = bytes[0];
        const std::uint32_t third = bytes[2];
        const std::uint32_t second = bytes[1];
        const std::uint32_t prefix = (first << 16) | (second << 8) | third;
        if (width == 3)
            return prefix;
        return (prefix << 8) | bytes[3];
    }
    default:return 0;
    }
}
BigKind big_kind(const std::uint8_t* archive) noexcept {
    if(archive[0]==192 && archive[1]==251)return BigKind::compact;
    if(archive[0]=='B' && archive[1]=='I' && archive[2]=='G')
        return archive[3]=='F'?BigKind::bigf:BigKind::variable;
    return BigKind::unknown;
}
std::uint32_t big_directory_size(const std::uint8_t* archive) noexcept {
    switch(big_kind(archive)) {
    case BigKind::compact:return big_read_field(archive+2,2)+4;
    case BigKind::bigf:case BigKind::variable:return big_read_field(archive+12,4);
    default:return 0;
    }
}
std::uint32_t big_entry_count(const std::uint8_t* archive) noexcept {
    switch(big_kind(archive)) {
    case BigKind::compact:return big_read_field(archive+4,2);
    case BigKind::bigf:case BigKind::variable:return big_read_field(archive+8,4);
    default:return 0;
    }
}
std::uint32_t big_trailer(const std::uint8_t* archive,int* version,std::uint32_t* value) noexcept {
    const auto* trailer=archive+big_directory_size(archive)-8;
    const auto alpha=[](std::uint8_t c){return (c>'@' && c<'[')||(c>'`' && c<'{');};
    const auto digit=[](std::uint8_t c){return c>'/' && c<':';};
    if(alpha(trailer[0]) && digit(trailer[1]) && digit(trailer[2]) && digit(trailer[3])) {
        if(version)*version=trailer[3]-5328+(trailer[2]+trailer[1]*10)*10;
        if(value)*value=big_read_field(trailer+4,4);
        return 8;
    }
    // This function leaves caller's output words unchanged when no trailer is present.
    return 0;
}
static int lower_character(char c) noexcept {

    // Other locale policies remain an integration dependency, not silently emulated.
    const int value=static_cast<signed char>(c);
    if(value>='A' && value<='Z')return value+('a'-'A');
    return value;
}
int big_name_compare(const char* left,const char* right) noexcept {
    int difference=lower_character(*left)-lower_character(*right);
    while(!difference && *left) {++left;++right;difference=lower_character(*left)-lower_character(*right);}
    return difference;
}
const char* big_find_entry(const std::uint8_t* archive,const char* name,int index,
                          std::uint32_t* offset,std::uint32_t* size) noexcept {
    const auto trailer_bytes=big_trailer(archive,nullptr,nullptr);
    const auto* directory_end=archive+big_directory_size(archive)-trailer_bytes;
    const auto* entry=archive+16;
    int offset_width=4,size_width=4;
    switch(big_kind(archive)) {
    case BigKind::compact:offset_width=size_width=3;entry=archive+6;break;
    case BigKind::variable:
        offset_width=static_cast<signed char>(archive[3])-'0';
        size_width=offset_width>4?4:offset_width;
        break;
    default:break;
    }
    int ordinal=0;
    while(entry<directory_end) {
        const char* entry_name=reinterpret_cast<const char*>(entry+offset_width+size_width);
        const bool match=name?big_name_compare(entry_name,name)==0:ordinal==index;
        if(match) {
            if(offset) {
                if(offset_width<5)*offset=big_read_field(entry,offset_width);
                else {
                    // Reads and discards the high part, then keeps the low dword.
                    (void)big_read_field(entry,offset_width-4);
                    *offset=big_read_field(entry+offset_width-4,4);
                }
            }
            if(size)*size=big_read_field(entry+offset_width,size_width);
            return entry_name;
        }
        entry=reinterpret_cast<const std::uint8_t*>(entry_name+std::strlen(entry_name)+1);
        ++ordinal;
    }
    if(offset)*offset=0;
    if(size)*size=0;
    return nullptr;
}
}
