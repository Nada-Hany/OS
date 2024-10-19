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
		uint32 *startptr = (uint32 *) daStart;
		*startptr=1;
		uint32 daEnd=daStart+initSizeOfAllocatedSpace-4;
		uint32 *endptr=(uint32 *)daEnd;
		*endptr=1;

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


	char *startptr = (char *) va-sizeof(uint32);

	uint32* header = (uint32*)startptr;
	*header = SizeandFlag;

	uint32* footer = (uint32*)(startptr+totalSize-sizeof(uint32));

	*footer = SizeandFlag;

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
			else if (elementSize - required_size < 16) {
				set_block_data(element, elementSize, 1);
			} else {
				struct BlockElement *new_free_block =(struct BlockElement *) ((char *) element+ required_size);
				set_block_data(element, required_size, 1);
				set_block_data(new_free_block, elementSize - required_size, 0);

			}
			return (void*) ((char*) element );

		}
	}
	void * new_block = sbrk(required_size);
	if (new_block == (void*) -1) {
		return NULL;
	} else {
		set_block_data(new_block, required_size, 1);
		return (void*) ((char*) new_block );
	}
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...

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
	//some data for my block
	struct BlockElement * my_block_ptr = (struct BlockElement *) va;
	uint32 my_block_size = get_block_size(va);
	//free my block only
	set_block_data(va,my_block_size, 0);
	//data of next block
	void * next_block =(void *) ((char*)va + my_block_size);
	uint32 next_block_size = get_block_size(next_block);
	//free next block if !allocated
	int8 is_next_free = is_free_block(next_block);
	if(is_next_free == 1){
		//cprintf("next is free\n");
		struct BlockElement * next_block_ptr = (struct BlockElement *) next_block;
		LIST_REMOVE(&freeBlocksList,next_block_ptr);
		LIST_REMOVE(&freeBlocksList,my_block_ptr);
		my_block_size=my_block_size+next_block_size;
		set_block_data(va, my_block_size, 0);
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
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...
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
