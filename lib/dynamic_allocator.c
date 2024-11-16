/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	    if (initSizeOfAllocatedSpace < 24) initSizeOfAllocatedSpace = 24;
		uint32 *startptr = (uint32 *) daStart;
		*startptr=1;
		uint32 daEnd=daStart+initSizeOfAllocatedSpace-4;
		uint32 *endptr=(uint32 *)daEnd;
		*endptr=1;
//		cprintf("entered init dynalloc\n");
		LIST_INIT(&freeBlocksList);
		set_block_data((void*)daStart+8, initSizeOfAllocatedSpace-8, 0);

}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	uint32 SizeandFlag = (totalSize);
	if (isAllocated) SizeandFlag++;

//	cprintf("stop 1\n");
	char *startptr = (char *) va-sizeof(uint32);

	uint32* header = (uint32*)startptr;
	*header = SizeandFlag;
//	cprintf("stop 2\n");
	uint32* footer = (uint32*)(startptr+totalSize-sizeof(uint32));

	*footer = SizeandFlag;
//	cprintf("stop 3\n");
	if (!isAllocated)
	{
		struct BlockElement* blockptr = (struct BlockElement *) (startptr+sizeof(uint32));
		struct BlockElement blockelement = {0};
		*blockptr = blockelement;
		struct BlockElement* it;
		bool found = 0;
		if(LIST_EMPTY(&freeBlocksList))
		{
			LIST_INSERT_HEAD(&freeBlocksList, blockptr);
			found=1;
		}
		else LIST_FOREACH(it, &freeBlocksList)
		{
			if(it > blockptr)
			{
				LIST_INSERT_BEFORE(&freeBlocksList, it, blockptr);
				found=1;
				break;
			}
		}
		if(!found)
		{
			LIST_INSERT_TAIL(&freeBlocksList, blockptr);
		}
	}
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0)
			size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE;
		if (!is_initialized) {
			uint32 required_size = size + 2 * sizeof(int) /*header & footer*/+ 2 * sizeof(int) /*da begin & end*/;
			uint32 da_start = (uint32) sbrk(
					ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
			uint32 da_break = (uint32) sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...
	if (size == 0) {
		return NULL;
	}

//	cprintf("block allocated with size %d\n", size);
	uint32 required_size = size + 2 * sizeof(int) /*header & footer*/ ;
	struct BlockElement *element;
	LIST_FOREACH(element ,&freeBlocksList)
	{
		uint32 elementSize = get_block_size(element);
		if (elementSize >= required_size) {
			LIST_REMOVE(&freeBlocksList, element);

			if (elementSize == required_size) {
				set_block_data(element, elementSize, 1);
			}
			//if size of new free block <16 min block size dont split
			else if (elementSize - required_size < 16) {
				set_block_data(element, elementSize, 1);
			} else {
				struct BlockElement *new_free_block =(struct BlockElement *) ((char *) element+ required_size);
				set_block_data(element, required_size, 1);
				set_block_data(new_free_block, elementSize - required_size, 0);
			}
//			cprintf("element found");
			return (void*) ((char*) element );
		}
	}
	uint32 numofPages = ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE ;
//	cprintf("pages needed %d\n", numofPages);
//	cprintf("old sbrk %\n",  (uint32)sbrk(0));
	void * new_block = sbrk(numofPages);
	if (new_block == (void*) -1) {
		return NULL;
	} else {
		void * ret = alloc_block_FF(size);
		return (char*) ret;
	}
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size) {
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...

	//doctor code
	{
		if (size % 2 != 0)
			size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE;
		if (!is_initialized) {
			uint32 required_size = size + 2 * sizeof(int) /*header & footer*/
			+ 2 * sizeof(int) /*da begin & end*/;
			uint32 da_start = (uint32) sbrk(
			ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE);
			uint32 da_break = (uint32) sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}

	//my code
	if (size == 0) {
		return NULL;
	}

	uint32 required_size = size + 2 * sizeof(int) /*header & footer*/;

	struct BlockElement *element;
	//uint32max= 0xFFFFFFFF
	uint32 best_block_size = 0xFFFFFFFF;
	struct BlockElement *best_block = NULL;
	LIST_FOREACH(element ,&freeBlocksList)
	{
		uint32 element_size = get_block_size(element);
		if (element_size >= required_size && element_size < best_block_size) {
			best_block = element;
			best_block_size = element_size;
		}
	}
	if (best_block != NULL) {

			LIST_REMOVE(&freeBlocksList, best_block);

			if (best_block_size == required_size) {
				set_block_data(best_block,best_block_size , 1);
			} else if (best_block_size - required_size < 16) {
				set_block_data(best_block, best_block_size, 1);
			} else {
				struct BlockElement *new_free_block =(struct BlockElement *) ((char *) best_block+ required_size);
				set_block_data(best_block, required_size, 1);
				set_block_data(new_free_block, best_block_size - required_size, 0);
			}
			return (void*) ((char*)best_block);
		}
	//if no free size
	uint32 numofPages = ROUNDUP(required_size, PAGE_SIZE) / PAGE_SIZE ;
	//	cprintf("pages needed %d\n", numofPages);
	//	cprintf("old sbrk %\n",  (uint32)sbrk(0));
		void * new_block = sbrk(numofPages);
		if (new_block == (void*) -1) {
			return NULL;
		} else {
			void * ret = alloc_block_BF(size);
			return (char*) ret;
		}
}

//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...
	if(va==NULL){
		return;
	}
	int8 is_myblock_free = is_free_block(va);
	if(is_myblock_free){
		return;
	}
	cprintf("stop 1\n");
	//some data for my block
	struct BlockElement * my_block_ptr = (struct BlockElement *) va;
	uint32 my_block_size = get_block_size(va);
	//free my block only
	set_block_data(va,my_block_size, 0);
	//data of next block
	void * next_block =(void *) ((char*)va + my_block_size);
	cprintf("next block add: %x, segbreak add: %x\n", (uint32)next_block, (uint32)sbrk(0));
	uint32 next_block_size = get_block_size(next_block);
	cprintf("stop 2.1\n");
	//free next block if !allocated
	int8 is_next_free = is_free_block(next_block);
	cprintf("is next free? %d\n", is_next_free);
	if(is_next_free == 1){
		//cprintf("next is free\n");
		struct BlockElement * next_block_ptr = (struct BlockElement *) next_block;
		LIST_REMOVE(&freeBlocksList,next_block_ptr);
		LIST_REMOVE(&freeBlocksList,my_block_ptr);
		my_block_size=my_block_size+next_block_size;
		set_block_data(va, my_block_size, 0);
		cprintf("stop 3\n");
	}


	//data of previous block
	uint32 prev_block_size = get_block_size((void*)((char*)va-4));
	void * prev_block = (void *) ((char*)va - prev_block_size);
	//free previous block if !allocated
	int8 is_prev_free = is_free_block(prev_block);
	if(is_prev_free == 1 && prev_block_size>0){
		//cprintf("prev is free with size %d\n", prev_block_size);
		struct BlockElement * prev_block_ptr = (struct BlockElement *) prev_block;
		LIST_REMOVE(&freeBlocksList,prev_block_ptr);
		LIST_REMOVE(&freeBlocksList,my_block_ptr);
		my_block_size=my_block_size+prev_block_size;
		set_block_data(prev_block,my_block_size, 0);
		cprintf("stop 4\n");
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...

	if (va == NULL && new_size == 0)
		return NULL;

	if (new_size == 0)
	{
		free_block(va);
		return NULL;
	}

	if (new_size % 2 != 0)
		new_size++;	//ensure that the size is even (to use LSB as allocation flag)
	if (new_size < DYN_ALLOC_MIN_BLOCK_SIZE)
		new_size = DYN_ALLOC_MIN_BLOCK_SIZE;

	if (va == NULL)
		return alloc_block_FF(new_size);


	// add header/footer sizes
	new_size = new_size + 8;
	uint32 old_size = get_block_size(va);

	if (old_size == new_size)
		return va;

	// next block data
	struct BlockElement * my_block_ptr = (struct BlockElement *) va;
	void * next_block =(void *) ((char*)va + old_size);
	struct BlockElement * next_block_ptr = (struct BlockElement *) next_block;
	uint32 next_block_size = get_block_size(next_block);
	int8 is_next_free = is_free_block(next_block);

	// reallocating with bigger size
	if (new_size > old_size)
	{
		// check if the next block can be merged or split
		if(is_next_free)
		{
			uint32 diff = new_size - old_size;
			// Split the next block
			if (next_block_size - diff >= 16)
			{
				LIST_REMOVE(&freeBlocksList, next_block_ptr);
				set_block_data(((char *)next_block_ptr+diff), next_block_size - diff, 0);
				set_block_data(va, new_size, 1);
				return va;
			}

			// merge without splitting
			else if (next_block_size - diff >= 0)
			{
				LIST_REMOVE(&freeBlocksList, next_block_ptr);
				set_block_data(va, next_block_size+old_size, 1);
				return va;
			}
		}
		// if next ! free || can't be merged
        //save the old data
        uint32 data_size = old_size - 8;
        char* old_ptr = (char*)va;
        char temp[data_size];

        for (uint32 i = 0; i < data_size; i++)
        {
            temp[i] = old_ptr[i];
        }

        // free the block and allocate in new place
		free_block(va);
		void* new_address = alloc_block_FF(new_size-8);

		if (new_address == NULL)
			return NULL;

		//copy data to new address
		char* new_ptr = (char*) new_address;
        for (uint32 i = 0; i < data_size; i++)
        {
            new_ptr[i] = temp[i];
        }
        return new_address;
	}

	// reallocating with smaller size
	else
	{
		uint32 diff = old_size - new_size;
		//if diff <16 can be merged with next block if next block is free
		//if diff >= 16 will be merged also
		if (diff >= 16)
		{
            set_block_data(va, new_size, 1);
            set_block_data(((char *)va + new_size), diff, 1);
            free_block(((char *)va + new_size));
		}
		else
		{
			if (is_next_free)
			{
				LIST_REMOVE(&freeBlocksList,next_block_ptr);
				set_block_data(va, new_size, 1);
				set_block_data(((char *)va + new_size), diff+next_block_size, 0);
			}
		}
		return va;
	}
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
