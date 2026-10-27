#include "nfs/graphics/fonts/fonts.hpp"
namespace nfs::fonts {

void initialize(){if(!state.context){state.context=initialize_font(builtin());kernel::register_exit(shutdown);}}

void shutdown(){if(state.context){release_font(state.context);state.context=nullptr;kernel::unregister_exit(shutdown);}}
}
