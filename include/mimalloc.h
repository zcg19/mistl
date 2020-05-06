#ifndef __MemPool_H__
#define __MemPool_H__


// memory block.
struct  MemwBlock_t;
MemwBlock_t* AllocMemwBlock(int size, int id);
void    CleanMemwBlock(MemwBlock_t * block);
void    FreeMemwBlock(MemwBlock_t * block);
bool    MemwBlockIsEmpty(MemwBlock_t * block);
bool    MemwBlockIsFully(MemwBlock_t * block);

void  * AllocBlockMemw(MemwBlock_t * block, int size);
void    FreeBlockMemw(MemwBlock_t * block, void * header);
void    DumpMemwBlock(MemwBlock_t * block);

void    ResetMemwPool();
void    DumpMemwPool();


// 
// memory alloc. 
// 基于块分配, 默认最小的块(MWP_BLOCK_MIN_SIZE)为 1M, 默认超过(MWP_NODE_HUGE_SIZE)不放在缓冲区中. 
// 可以根据需要修改 MWP_BLOCK_MIN_SIZE. 
//
void  * MwAlloc(unsigned int size);
void    MwFree (void * pMemory);
void  * MwRealloc(void* mwOld, int size);


#endif
