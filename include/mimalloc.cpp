#include "micomm.h"
#include "mimalloc.h"


DESCLARE_NAMESPACE_MISTD_BEGIN
enum  enumNodeState 
{ 
	MWN_STATUS_FREE = 0x0, 
	MWN_STATUS_FREE_END, 
	MWN_STATUS_ALLOC, 
	MWN_STATUS_HUGE = 0x4, 
};

enum enumNodeMergeState
{
	MWB_STATUS_MERGER_NOTHING  = 0x0, 
	MWB_STATUS_MERGER_TWO_FRONT, 
	MWB_STATUS_MERGER_TWO_NEXT, 
	MWB_STATUS_MERGER_THREE, 
};

typedef struct MemwNodeHeader_t
{
	unsigned char   status:3;
	unsigned char   bid:5;
	unsigned int    front;
	unsigned int    next;
	//int           free_next;                      // hide.
}MemwNodeHeader_t;

typedef struct MemwBlock_t
{
	union
	{
		int         status;
		struct 
		{
			char    id;
			#define MWB_STATUS_MASK                 0xFF
			#define MWB_STATUS_ACTIVE               0x00
			#define MWB_STATUS_DROP                 0x01
			char    flag;
		};
	};

	unsigned int    full_size;                      // actually size(drop block_header), it is also memeory array size.
	unsigned int    alloc_size;                     // mean has been used by user, unit is byte
	unsigned int    free_size;                      // mean can used by user, unit is byte
	unsigned int    free_index;                     // for impove efficiency, when iterate
	unsigned int    free_count;                     // ...
}MemwBlock_t;


// node.
#define MWN_INDEX_INVALID                           -1
#define MWN_MIN_DATA_SIZE                           4
#define MWN_HEADER_SIZE                             (sizeof(MemwNodeHeader_t))
#define MWN_IS_FREE(_header)                        ((_header->status & 0x3) != MWN_STATUS_ALLOC)
#define MWN_IS_FREE_END(_header)                    (_header->status == MWN_STATUS_FREE_END)
#define MWN_SET_FREE(_header)                       (_header->status  = MWN_STATUS_FREE)
#define MWN_SET_FREE_END(_header)                   (_header->status  = MWN_STATUS_FREE_END)
#define MWN_SET_ALLOC(_header)                      (_header->status  = MWN_STATUS_ALLOC)
#define MWN_SET_HUGE(_header)                       (_header->status |= MWN_STATUS_HUGE)
#define MWN_DATA_SIZE(_header, _index)				((_header)->next - (_index) - MWN_HEADER_SIZE)
#define MWN_NODE_SIZE(_size)						((_size) + MWN_HEADER_SIZE)
#define MWN_NEXT_FREE_INDEX(_header)                (*(unsigned int*)(_header+1))
#define MWN_MIN_NODE_SIZE                           (MWN_HEADER_SIZE + MWN_MIN_DATA_SIZE)

#define MW_HEADER_INIT(_nheader, _block_id, _front, _next, _status) \
	{ \
		(_nheader)->status = _status; \
		(_nheader)->bid    = _block_id; \
		(_nheader)->front  = _front; \
		(_nheader)->next   = _next; \
	}

// block.
#define MWB_HEADER_SIZE                             (sizeof(MemwBlock_t) + MWN_HEADER_SIZE)
#define MWB_MIN_SIZE                                (MWB_HEADER_SIZE + MWN_HEADER_SIZE)
#define MWB_FULL_ALLOC_SIZE(_block)                 ((_block)->full_size + MWB_HEADER_SIZE)

#define MWB_FLAG(_block)							((_block)->flag)
#define MWB_STATUS(_block)							(MWB_FLAG(_block) &  MWB_STATUS_DROP)
#define MWB_IS_DROP(_block)							(MWB_FLAG(_block) &  MWB_STATUS_DROP)
#define MWB_SET_ACTIVE(_block)						(MWB_FLAG(_block) &= ~MWB_STATUS_DROP)
#define MWB_SET_DROP(_block)						(MWB_FLAG(_block) |= MWB_STATUS_DROP)

#define MWB_NODE_INDEX_0							(MWB_HEADER_SIZE)
#define MWB_NODE_INDEX_F(_block)					((_block)->full_size)
#define MWB_NODE_INDEX(_block, _node)				(unsigned int)((char*)(_node) - (char*)(_block))
#define MWB_NODE_HEADER(_block, _idx)               ((MemwNodeHeader_t*)((char*)(_block) + (_idx)))

#define MWB_NODE_HEADER_0(_block)                   MWB_NODE_HEADER(_block, MWB_NODE_INDEX_0)
#define MWB_NODE_HEADER_NEXT(_block, _node)         MWB_NODE_HEADER(_block, (_node)->next)
#define MWB_NODE_HEADER_FRONT(_block, _node)        MWB_NODE_HEADER(_block, (_node)->front)
#define MWB_NODE_HEADER_NEXT_EX(_node, _idx)        ((MemwNodeHeader_t*)((char*)_node + (_node)->next - _idx))
#define MWB_NODE_HEADER_FRONT_EX(_node, _idx)       ((MemwNodeHeader_t*)((char*)_node + _idx - (_node)->front))

#define MWB_NODE_IS_BEGIN(_block, _node)            (MWB_NODE_INDEX(_block, _node) == MWB_NODE_INDEX_0)
#define MWB_NODE_IS_BEGIN_EX(_index)                ((_index) == MWB_NODE_INDEX_0)
#define MWB_NODE_IS_END(_block, _node)              ((_node)->next >= (_block)->full_size)

// tranfer from 'real memory address' to 'memory_wrapper header' == get 'mwheader'
#define MWM_NODE_HEADER(_memw)						((MemwNodeHeader_t*)((char*)(_memw) - MWN_HEADER_SIZE))
#define MWN_DATA(_node)								((char*)(_node + 1))		// ((char*)(mwheader) + MWN_HEADER_SIZE)
#define MWB_HEADER(_node, _idx)						((MemwBlock_t*)((char*)(_node) - (_idx)))


// init.
#define MW_BLOCK_BEGIN_HEADER_INIT(_block, _block_id)		\
	{ \
		MW_HEADER_INIT(MWB_NODE_HEADER_0(_block), _block_id, (_block)->full_size, (_block)->full_size, MWN_STATUS_FREE_END) \
	}

#define MW_BLOCK_END_HENADER_INIT(_block, _block_id)		\
	{ \
		MW_HEADER_INIT(MWB_NODE_HEADER(_block, (_block)->full_size), _block_id, MWB_HEADER_SIZE, MWB_HEADER_SIZE, MWN_STATUS_ALLOC) \
	}

#define MW_BLOCK_INITD(_block, _size, _block_id)			\
	{ \
		(_block)->full_size  = _size - MWB_HEADER_SIZE; \
		(_block)->alloc_size = MWN_HEADER_SIZE; \
		(_block)->free_size  = _size - MWB_MIN_SIZE; \
		(_block)->id         = _block_id; \
		(_block)->free_index = MWB_HEADER_SIZE; \
		(_block)->free_count = 1; \
		MW_BLOCK_BEGIN_HEADER_INIT(_block, _block_id); \
		MW_BLOCK_END_HENADER_INIT(_block, _block_id); \
	}

#define MWB_INSERT_NEW_HEADERM(_block, _front, _new_idx, _new_free_idx) \
	{ \
		MemwNodeHeader_t * __new = MWB_NODE_HEADER(_block, _new_idx); \
		__new->front         = MWB_NODE_INDEX(_block, _front); \
		__new->next          = (_front)->next; \
		__new->status        = (_front)->status; \
		__new->bid           = (_block)->id; \
		(_front)->next       = _new_idx; \
		MWN_NEXT_FREE_INDEX(__new) = _new_free_idx; \
		MWB_NODE_HEADER_NEXT(_block, __new)->front = _new_idx; \
	}

// merge 'front, current, next'
#define MWB_MERGE_HEADER3(_front, _current, _next)	\
	{ \
		MWB_NODE_HEADER_NEXT_EX(_next, (_current)->next)->front = (_current)->front; \
		(_front)->next = (_next)->next; \
	}

// merge 'front, current'
#define MWB_MERGE_HEADER2F(_front, _current, _next)	\
	{ \
		(_front)->next = (_current)->next; \
		if((_next) != NULL) (_next)->front = (_current)->front; \
	}

// merge 'current, next'
#define MWB_MERGE_HEADER2N(_front, _current, _next)	\
	{ \
		MWB_NODE_HEADER_NEXT_EX(_next, (_current)->next)->front = (_next)->front; \
		(_current)->next = (_next)->next; \
	}

// insert new free node.
#define MWB_FREE_NODE_INSERT(_block, _current, _current_index) \
	{ \
		if(_block->free_index == MWN_INDEX_INVALID) \
		{ \
			_block->free_index = _current_index; \
			MWN_SET_FREE_END(_current); \
		} \
		else \
		{ \
			MemwNodeHeader_t * __tmp = 0; \
			unsigned int __current_index_copy = _current_index; \
			if(__current_index_copy < _block->free_index) __current_index_copy += _block->full_size; /* overflow ??? */ \
			for(unsigned int __i = _block->free_index, __j = 0, __j1;; __i = __j) \
			{ \
				__tmp = MWB_NODE_HEADER(_block, __i); \
				if(MWN_IS_FREE_END(__tmp))     { MWN_SET_FREE(__tmp); MWN_SET_FREE_END(_current); break; } \
				__j1  = __j = MWN_NEXT_FREE_INDEX(__tmp); \
				if(__j < _block->free_index) __j1 += _block->full_size; \
				if(__current_index_copy < __j1) { MWN_SET_FREE(_current); MWN_NEXT_FREE_INDEX(_current) = __j; break; } \
			} \
			miassert(__tmp); MWN_NEXT_FREE_INDEX(__tmp) = _current_index; \
		} \
	}

#define MWB_FREE_NODE_REPLACE_NEXT(_block, _current, _next, _current_index) \
	{ \
		int __next_index  = MWB_NODE_INDEX(_block, _next); \
		_current->status = _next->status; \
		MWN_NEXT_FREE_INDEX(_current) = MWN_NEXT_FREE_INDEX(_next); \
		\
		miassert(_block->free_index != MWN_INDEX_INVALID); \
		if(_block->free_index == __next_index) { _block->free_index = _current_index; } \
		else \
		for(unsigned int __i = _block->free_index, __j = 0; ; __i = __j) \
		{ \
			MemwNodeHeader_t * __tmp = MWB_NODE_HEADER(_block, __i); \
			miassert(!MWN_IS_FREE_END(__tmp)); __j = MWN_NEXT_FREE_INDEX(__tmp); \
			if(__next_index == __j) { MWN_NEXT_FREE_INDEX(__tmp) = _current_index; break; } \
		} \
	}

#define MWB_FREE_NODE_DELETE_NEXT(_block, _front, _next) \
	{ \
		if(MWN_IS_FREE_END(_next)) MWN_SET_FREE_END(_front); \
		MWN_NEXT_FREE_INDEX(_front) = MWN_NEXT_FREE_INDEX(_next); \
		\
		if(_block->free_index == MWB_NODE_INDEX(_block, _next)) \
		{ \
			MemwNodeHeader_t * __tmp = MWB_NODE_HEADER(_block, _block->free_index); \
			_block->free_index = MWN_NEXT_FREE_INDEX(__tmp); \
		} \
	}


static bool AssertMemoryIdx(MemwBlock_t * block, int index)
{
	for(unsigned int i = MWB_NODE_INDEX_0; i < block->full_size;)
	{
		if(index == i) return true;
		i = MWB_NODE_HEADER(block, i)->next;
	}

	MILOG("[error]: memw block, assert -->wrong 'memory_idx = %d'!\n", index);
	return false;
}

MemwBlock_t * AllocMemwBlock(int size, int id)
{
	miassert(size > MWB_MIN_SIZE);
	MemwBlock_t* block = (MemwBlock_t*)__mpmalloc(size);
	if(!block)
	{
		MILOG("[error]: memw block, real alloc failed-->%d\n", size);
		return 0;
	}

	MW_BLOCK_INITD(block, size, id);
	return block;
}

void   CleanMemwBlock(MemwBlock_t * block)
{
	if(block) MW_BLOCK_INITD(block, MWB_FULL_ALLOC_SIZE(block), block->id);
}

void   FreeMemwBlock(MemwBlock_t * block)
{
	if(block) __mpfree(block);
}

bool   MemwBlockIsEmpty(MemwBlock_t * block)
{
	miassert(block);
	return (block->free_size == (block->full_size - MWN_HEADER_SIZE));
}

bool   MemwBlockIsFully(MemwBlock_t * block)
{
	miassert(block);
	return (block->free_size == 0);
}

void * AllocBlockMemw(MemwBlock_t * block, int size)
{
	miassert(block && size > 0);

	MemwNodeHeader_t * front = 0, * current = 0, * next = 0;
	int node_size = 0, current_index = 0, current_free_index = 0, new_index = 0, is_alloc = 0;

	// no free space.
	if(block->free_index == MWN_INDEX_INVALID) return 0;
	if(size < MWN_HEADER_SIZE) size = MWN_HEADER_SIZE;
	current = MWB_NODE_HEADER(block, block->free_index);

	while(current)
	{
		miassert(MWN_IS_FREE(current));
		current_index = MWB_NODE_INDEX(block, current);  current_free_index = MWN_NEXT_FREE_INDEX(current);
		next  = MWN_IS_FREE_END(current) ? 0 : MWB_NODE_HEADER(block, current_free_index);
		if((node_size = MWN_DATA_SIZE(current, current_index)) > size) { is_alloc = 1; break; }
		front = current; current = next;
	}

	// free space is not enough;
	if(!is_alloc) return 0;

	// 4 types.
	if(node_size > (size + (int)MWN_MIN_NODE_SIZE))
	{
		node_size = MWN_NODE_SIZE(size); new_index = current_index + node_size;
		MWB_INSERT_NEW_HEADERM(block, current, new_index, current_free_index);

		miassert(MWN_IS_FREE(MWB_NODE_HEADER(block, new_index)));
		if(front) MWN_NEXT_FREE_INDEX(front) = new_index;
		if(block->free_index == current_index) block->free_index = new_index;
	}
	else
	{
		if(front)
		{
			front->status = current->status;
			MWN_NEXT_FREE_INDEX(front) = current_free_index;
		}

		miassert(block->free_count > 0); block->free_count--;
		if(block->free_index == current_index) block->free_index = MWN_IS_FREE_END(current) ? MWN_INDEX_INVALID : current_free_index;
	}

	block->alloc_size += node_size; block->free_size  -= node_size;
	MWN_SET_ALLOC(current);

	return MWN_DATA(current);
}

void   FreeBlockMemw(MemwBlock_t * block, void * ptr)
{
	MemwNodeHeader_t * front = 0, * next = 0, * current = 0;
	int                current_index = 0, current_size = 0,  merge_flag = MWB_STATUS_MERGER_NOTHING;

	miassert(block != 0 && ptr != 0);
	miassert(ptr >= ((char*)block + MWB_MIN_SIZE));
	miassert(ptr <  ((char*)block + MWB_HEADER_SIZE + block->full_size));

	if(MWN_IS_FREE((current = MWM_NODE_HEADER(ptr))))
	{
		miassert(0);
		MILOG("[error]: memw block, free -->this memory have been free!\n");
		return ;
	}

	current_index = MWB_NODE_INDEX(block,  current); miassert(AssertMemoryIdx(block, current_index));
	current_size  = MWN_DATA_SIZE(current, current_index);

	if(!MWB_NODE_IS_BEGIN_EX(current_index))
	{
		front = MWB_NODE_HEADER_FRONT(block, current);
		if(!MWN_IS_FREE(front)) front = 0;
		else 
		{
			merge_flag   += MWB_STATUS_MERGER_TWO_FRONT;
			current_size += MWN_HEADER_SIZE;
		}
	}

	if(!MWB_NODE_IS_END(block, current))
	{
		next  = MWB_NODE_HEADER_NEXT(block, current);
		if(MWN_IS_FREE(next))
		{
			merge_flag   += MWB_STATUS_MERGER_TWO_NEXT;
			current_size += MWN_HEADER_SIZE;
		}
	}

	switch(merge_flag)
	{
	case MWB_STATUS_MERGER_NOTHING:
		MWB_FREE_NODE_INSERT(block, current, current_index);
		block->free_count++;
		break;

	case MWB_STATUS_MERGER_TWO_FRONT:
		MWB_MERGE_HEADER2F(front, current, next);
		break;

	case MWB_STATUS_MERGER_TWO_NEXT:
		miassert(next);
		MWB_FREE_NODE_REPLACE_NEXT(block, current, next, current_index);
		MWB_MERGE_HEADER2N(front, current, next);
		break;

	case MWB_STATUS_MERGER_THREE:
		miassert(front);
		miassert(next);
		miassert(block->free_count > 0);
		block->free_count--;
		MWB_FREE_NODE_DELETE_NEXT(block, front, next);
		MWB_MERGE_HEADER3(front, current, next);
		break;

	default:
		miassert(0);
		break;
	}

	block->alloc_size -= current_size;
	block->free_size  += current_size;
}

void   DumpMemwBlock(MemwBlock_t * block)
{
	int free_size      = 0, i = MWB_NODE_INDEX_0, free_count = 0;
	int alloc_size     = 0, alloc_head_size = 0;

	MILOG("--------------------memw block begin--------------------（%p,%d)\n", block, block->id);
	MILOG("--- block(%u), phead(%u), bhead(%u), (%d)\n", MWB_FULL_ALLOC_SIZE(block), MWB_HEADER_SIZE, MWN_HEADER_SIZE, block->id);
	MILOG("      idx |  front |    next |    size |    status| \n");
	do
	{
		MemwNodeHeader_t * node = MWB_NODE_HEADER(block, i);
		MILOG("%10d%10d%10d%10d%10d\n", i, node->front, node->next, MWN_DATA_SIZE(node, i), node->status);

		if(MWN_IS_FREE(node)) free_size  += MWN_DATA_SIZE(node, i) > 0 ? MWN_DATA_SIZE(node, i) : 0;
		else                  alloc_size += MWN_DATA_SIZE(node, i) > 0 ? MWN_DATA_SIZE(node, i) : 0;

		alloc_head_size += MWN_HEADER_SIZE; i = node->next;
	}while(i != MWB_NODE_INDEX_0);

	MILOG("free_size = %d, alloc_size = %d\n"
		"--- head: %d, data: %d, count = %d, \n"
		"--- free_idx: %d, free_count: %d\n" 
		"--- free_list: ", 
		free_size, (alloc_size + alloc_head_size), 
		alloc_head_size, alloc_size, alloc_head_size/sizeof(MemwNodeHeader_t), 
		block->free_index, block->free_count);

	// free node list.
	for(int j = block->free_index; j != MWN_INDEX_INVALID; )
	{
		MILOG("%d,", j); free_count++;
		MemwNodeHeader_t * node = MWB_NODE_HEADER(block, j);
		if(MWN_IS_FREE_END(node)) break;
		j = MWN_NEXT_FREE_INDEX(node);
	}
	miassert(free_count == block->free_count);
	MILOG("\n--------------------memw block end  --------------------\n\n");
}


// memory pool.
#ifndef MWP_BLOCK_MIN_SIZE
#define MWP_BLOCK_MIN_SIZE                          (1024 * 1024)
#endif
#define MWP_BLOCK_MAX_SIZE                          (512 * MWP_BLOCK_MIN_SIZE)		// 512M
#define MWP_BLOCK_MAX_NUM                           (sizeof(g_nMemBlockNumTable)/sizeof(g_nMemBlockNumTable[0]))
#define MWP_BLOCK_ACTIVE_NUM                        2
#define MWP_NODE_MIN_SIZE(_id)                      (_id*MWN_MIN_DATA_SIZE)
#define MWP_NODE_MAX_SIZE(_sz)                      ((_sz)/128)
#define MWP_NODE_HUGE_SIZE                          MWP_NODE_MAX_SIZE(MWP_BLOCK_MAX_SIZE)

// 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 = 512   ==> 1024
// max_size = 1024 * MEM_BLOCK_SIZE_UNIT.
static const int g_nMemBlockNumTable[]  = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512}; // 10.

static struct MemwPool_t
{
	MemwBlock_t   *	block_list[MWP_BLOCK_MAX_NUM];
	int             block_huge_num;
	union
	{
		int			num;
		struct
		{
			int		block_active_list_num : 4;
			int		block_drop_list_num : 4;
			int		block_active_list_idx : 4;						// 总是递增，drop_list_idx则不需要记录，因为设置为drop的idx不会转化成active
			int		block_reserved : 8;
			int		block_total_size : 12;							// unit is '1M'
		};
	};
}
g_mpool = {0};


static MemwBlock_t* CreateMemBlock(int id, int size)
{
	MemwBlock_t * block;

	block = AllocMemwBlock(size, id); miassert(block != 0);
	if(!block)   return 0;

	MWB_SET_ACTIVE(block); block->id = id;
	g_mpool.block_active_list_num++;
	g_mpool.block_total_size += size/(MWP_BLOCK_MIN_SIZE);
	return block;
}

static void DestoryMemBlock(int id)
{
	if(!MWB_IS_DROP(g_mpool.block_list[id]))
	{
		MWB_SET_DROP(g_mpool.block_list[id]);
		g_mpool.block_active_list_num--;
		g_mpool.block_drop_list_num++;
		g_mpool.block_active_list_idx++;									// bugger???
	}
}

void   DumpMemwPool()
{
	MILOG("-----------------memw pool begin--------------------\n");
	MILOG("pool:   num |    size |    active(idx, num) |    drop(idx, num) |   huge(idx, size)\n");
	MILOG("%10d%10d%20d, %d%17c, %d, %10d, %u\n", 
		g_mpool.block_active_list_num + g_mpool.block_drop_list_num, 
		g_mpool.block_total_size, 
		g_mpool.block_active_list_idx, g_mpool.block_active_list_num, 
		'-', g_mpool.block_drop_list_num, g_mpool.block_huge_num, MWP_NODE_HUGE_SIZE);

	MILOG("========\n");
	MILOG("pool:   idx |     size(total, free) |    status(active=0, drop=1)| \n");
	for(int i = 0; i < MWP_BLOCK_MAX_NUM; i++)
	{
		if(g_mpool.block_list[i] != 0)
		{
			MILOG("%10d%17d, %d%20d\n", 
				g_mpool.block_list[i]->id, 
				MWB_FULL_ALLOC_SIZE(g_mpool.block_list[i]), g_mpool.block_list[i]->free_size, 
				MWB_STATUS(g_mpool.block_list[i]));
			//DumpMemwBlock(g_mpool.block_list[i]);
		}
		else
		{
			MILOG("%10d%17c, %5c%20c\n", i, '-', '-', '-');
		}
	}
	MILOG("-----------------memw pool end (%)--------------------\n", sizeof(MemwPool_t));
}

void   ResetMemwPool()
{
	for(int i = 0; i < MWP_BLOCK_MAX_NUM; i++)
	{
		if(g_mpool.block_list[i] != 0)
		{
			FreeMemwBlock(g_mpool.block_list[i]);
			g_mpool.block_list[i] = 0;
		}
	}

	g_mpool.num = 0;
}

void * MwAlloc(unsigned int size)
{
	if(size > MWP_NODE_HUGE_SIZE)
	{
		void * p = __mpmalloc(size+MWN_HEADER_SIZE);
		if(!p) return 0;

		MemwNodeHeader_t * node = (MemwNodeHeader_t*)p;
		node->status = MWN_STATUS_HUGE;
		node->bid    = 0;
		node->front  = g_mpool.block_huge_num++;
		node->next   = 0;
		return MWN_DATA(node);
	}

	for(int i = g_mpool.block_active_list_idx; i < MWP_BLOCK_MAX_NUM; i++)
	{
		MemwBlock_t * block;
		int    block_size, block_node_max_size;
		char * ptr = 0;

		block_size =  g_nMemBlockNumTable[i] * MWP_BLOCK_MIN_SIZE;
		block_node_max_size = MWP_NODE_MAX_SIZE(block_size);
		if(block_node_max_size < MWP_NODE_MIN_SIZE(i)) block_node_max_size = MWP_NODE_MIN_SIZE(i);
		if((int)size > block_node_max_size) continue;

		block = g_mpool.block_list[i];
		if(!block)
		{
			block = g_mpool.block_list[i] = CreateMemBlock(i, block_size);
			if(i > 0 && g_mpool.block_list[i-1]) DumpMemwBlock(g_mpool.block_list[i-1]);
			MILOG("[test]: memw pool, new block -->(%d,%d,%d), (%d)\n", i, block_size, block_node_max_size, size);
		}

		if(!block)
		{
			MILOG("[error]: mimem pool, create block failed -->%d,%d\n", i, block_size);
			return 0;
		}

		if(ptr = (char*)AllocBlockMemw(block, size))
		{
			//MILOG_DEBUG("[debug]: mimem ++++++++++++++%p,%d, alloc(%d)\n", ptr, size, i);
			//DumpMemwBlock(block);
			return ptr;
		}
	}

	MILOG("[error]: memw pool, malloc failed -->block size reach to max limit!(2)\n");
	g_mpool.block_active_list_idx = MWP_BLOCK_MAX_NUM - 1;				// ... ???
	return 0;
}

void   MwFree(void * ptr)
{
	MemwNodeHeader_t * node  = 0;
	MemwBlock_t      * block = 0;

	miassert(ptr);
	node  = MWM_NODE_HEADER(ptr);

	miassert(node);
	miassert(node->bid >= 0 && node->bid < sizeof(g_mpool.block_list)/sizeof(g_mpool.block_list[0]));
	block = g_mpool.block_list[node->bid];

	if(node->status == MWN_STATUS_HUGE)
	{
		__mpfree(node);
		g_mpool.block_huge_num--;
		return ;
	}

	//MILOG_DEBUG("[debug]: mimem --------------%p, free(%d,%d)\n", ptr, block->id, MWB_NODE_INDEX(block, node));
	FreeBlockMemw(block, ptr);
	//DumpMemwBlock(block);

	if(MWB_IS_DROP(block) && MemwBlockIsEmpty(block))
	{
		MILOG_DEBUG("[notice]: free block, idx = %d\n", block->id);
		g_mpool.block_list[block->id] = 0;
		FreeMemwBlock(block);
		g_mpool.block_drop_list_num--;
	}
}

void * MwRealloc(void * old, int size)
{
	// ???
	// new    'new'
	// copy   'old' --> 'new'
	// delete 'old'

	MILOG_DEBUG("[notice]: memw pool, realloc -->sorry, it is not implemented! ...\n");
	return 0;
}

int    micalc_hash_string(miuchar * str, int len)
{
	// php ?
	unsigned  long h = 0, g;
	miuchar * begin = (miuchar*)str, *end = begin + len;

	while(begin < end)
	{
		h = (h << 4) + *begin++; 
		if((g = (h & 0xF0000000)))
		{ 
			h = h ^ (g >> 24); 
			h = h ^ g; 
		} 
	} 
	return h;
}


DESCLARE_NAMESPACE_MISTD_END
#ifndef MILOG_FILE
#define MILOG_FILE    "d:/mylog/mistl.log"
#endif

#pragma warning(push)
#pragma warning(disable: 4996)
FILE  * g_milog_file = fopen(MILOG_FILE, "wb");
#pragma warning(pop)
