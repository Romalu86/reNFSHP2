#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::methods {

std::uint8_t cpu_program(Dword* command){while(*command){if((*command>>16)==4)return 1;command+=*command&65535;}return 0;}

Dword* find_command(Dword kind,Dword* command){while(*command){if((*command>>16)==kind)return command;command+=*command&65535;}return nullptr;}
}
