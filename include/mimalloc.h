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
// ���ڿ����, Ĭ����С�Ŀ�(MWP_BLOCK_MIN_SIZE)Ϊ 1M, Ĭ�ϳ���(MWP_NODE_HUGE_SIZE)�����ڻ�������. 
// ���Ը�����Ҫ�޸� MWP_BLOCK_MIN_SIZE. 
//
void  * MwAlloc(unsigned int size);
void    MwFree (void * pMemory);
void  * MwRealloc(void* mwOld, int size);


#endif
