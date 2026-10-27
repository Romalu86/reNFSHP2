#pragma once
#include "nfs/core/types/kernel.hpp"
#include <cstdarg>
namespace nfs::kernel::print {
using Destination=void(__cdecl*)(int,const char*);
struct Device {const char* name;Destination write;Dword flags;};
struct Channel {const char* name;Dword flags;};
static_assert(sizeof(Device)==12&&sizeof(Channel)==8);
extern Device devices[8];extern Channel channels[64];
int check_console_write(const char*,...);
void initialize();void reset();
void write_v(int,const char*,va_list);void write(int,const char*,...);
void set_device(int,Dword);void device_flag(int,Dword);void parse_devices(const char*);
void lower_copy(const char*,char*,int);int prefix_length(const char*,const char*);
int find_device(const char*);void register_device(int,const char*,Destination);
void set_channel(int,int);void channel_flag(int,int);void parse_channels(const char*);
int find_channel(const char*);Dword enabled(int);void register_channel(int,const char*);
void direct(int,int,const char*);
void open_log(const char*);void close_log();
}
