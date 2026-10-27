#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::fonts {
using kernel::Dword;using Byte=std::uint8_t;
struct Font {
    Dword magic,bytes;std::uint16_t unknown_8,count;Dword flags;
    std::uint16_t unknown_10;Byte ascent,descent;Dword glyph_offset,kerning_offset;
    Dword unknown_1c,unknown_20,height,unknown_28;float unknown_2c,unknown_30,unknown_34,scale_x,scale_y;
};
struct DrawItem {const Byte* glyph;float x,y;};
using Draw=void(__cdecl*)(Font*,const Byte*,float,float);
using FontCall=void(__cdecl*)(Font*);
struct Callbacks {Draw draw;FontCall begin,end,initialize,release;};
using Batch=void(__cdecl*)(Font*,const DrawItem*,int);
struct Settings {Dword size;Batch batch;Dword unknown_8[5];};
struct State {Settings settings;Font* context;};
static_assert(sizeof(Font)==64&&offsetof(Font,glyph_offset)==20&&offsetof(Font,height)==36&&offsetof(Font,scale_x)==56);
static_assert(sizeof(Callbacks)==20&&sizeof(Settings)==28&&offsetof(State,context)==28&&sizeof(DrawItem)==12);
extern const Callbacks* callbacks;extern State state;
Font* builtin();Font* initialize_font(Font*);void release_font(Font*);void set_callbacks(const Callbacks*);
void configure(const Settings*);void initialize();void shutdown();
const Byte* find_code(Dword,const Byte*,int,int);int alternate_case(int);
const Byte* find_kerning(int,const Byte*,Dword);const Byte* find_pair(int,const Byte*,Dword,Dword);
int kerning(const Font*,const Byte*,Dword);Dword height(const Font*);
void draw_text(Font*,float,float,const Byte*);void draw_wide(Font*,float,float,const std::uint16_t*);
void measure(Font*,const Byte*,float*,float*,float*,float*);
}
