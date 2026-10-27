#include "nfs/core/types/kernel.hpp"
namespace nfs::kernel {
static FreeBlock* free_node(Block* b){return reinterpret_cast<FreeBlock*>(b);}
// only AL is defined by its contract.
std::uint8_t memory_validate_free_node(Block* b){if(!IsBadReadPtr(b,24)&&(b->magic==16966||b->magic==16979))return 1;memory_error=1;return 0;}

std::uint8_t memory_validate_block(Block* b){if(b->magic==16979){if(b->flags&32768)return 1;memory_error=2;return 0;}
    if(b->magic==16966){if(b->flags&16384){if(memory_validate_free_node(free_node(b)->next_free))return memory_validate_free_node(free_node(b)->previous_free);}return 0;}
    if(b->magic==16973){if(b->flags&8192){auto* p=reinterpret_cast<unsigned char*>(b)+16+b->size;
        Dword marker=(Dword(p[0])<<24)|(Dword(p[1])<<16)|(Dword(p[2])<<8)|p[3];if(marker!=1111838276){memory_error=3;return 0;}}return 1;}
    memory_error=4;return 0;}

std::uint8_t memory_validate_links(Block* b){if(b->magic==16979){if(!(b->flags&32768)){memory_error=2;return 0;}return 1;}
    Block* previous_link;if(b->magic==16966){if(!(b->flags&16384))return 0;
        if(free_node(free_node(b)->next_free)->previous_free!=b){memory_error=5;return 0;}
        previous_link=free_node(free_node(b)->previous_free)->next_free;
    }else{if(b->magic!=16973){memory_error=4;return 0;}
        if(b->next->previous!=b){memory_error=5;return 0;}previous_link=b->previous->next;}
    if(previous_link!=b){memory_error=5;return 0;}return 1;}

std::uint8_t memory_validate_free_list(Block* anchor){Block* node=anchor;
    do{node=free_node(node)->next_free;if(node->magic!=16966)break;}while(node->flags&16384);
    if(node->magic==16979){do{node=free_node(node)->previous_free;if(node->magic!=16966)break;}while(node->flags&16384);
        if(node==anchor)return 1;}memory_error=6;return 0;}

std::uint8_t memory_validate_pool(Dword id){Pool* p=pools[id&63];if(p->locked)lock_enter(&p->lock);Block* node=p->first;std::uint8_t valid;
    for(;;){if(!memory_validate_block(node)||!memory_validate_links(node))valid=0;else{node=node->next;valid=1;}
        if(!node||!valid)break;}if(valid)valid=memory_validate_free_list(&p->anchor.block);if(p->locked)lock_leave(&p->lock);return valid;}

void memory_validate_all(){std::uint8_t valid=1;for(int i=0;i<64;++i){if(!valid)return;if(pools[i])valid=memory_validate_pool(i);}}
}
