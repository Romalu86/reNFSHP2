#include "nfs/graphics/fonts/fonts.hpp"
namespace nfs::fonts {

// No code is loaded or executed from this array.
alignas(4) static Byte font_data[]={
#include "font_builtin.inc"
};
Font* builtin(){return reinterpret_cast<Font*>(font_data+512);}
}
