#include "nfs/input/directinput/directinput.hpp"
namespace nfs::input {
#include "directinput_formats.inc"
// Five maps have a 96-byte stride; only 91 keys are searched.
Dword scan_code(Dword key){if(static_cast<int>(key)>255)key=static_cast<Dword>(static_cast<int>(key|32768)>>8);
    for(int map=0;map<5;++map)for(int scan=0;scan<91;++scan)if(key_maps[map][scan]==key)return scan;return 0;}
// ALT takes precedence over CTRL, SHIFT and CAPSLOCK.
Dword translate_state(const std::uint8_t* state,std::uint8_t caps,int scan){int map;
    if(state[56]||state[184])map=4;else if(state[29]||state[157])map=3;
    else if(state[42]||state[54])map=1;else map=caps?2:0;
    Dword key=key_maps[map][scan];if(key>127){if(key>143)key&=127;key<<=8;}return key;}
}
