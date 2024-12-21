#include <inc/lib.h>
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
uint32 virtual_addresses_pages_num [NUM_OF_UHEAP_PAGES];
uint32 slave_to_master[NUM_OF_UHEAP_PAGES];
uint8 page_marked[NUM_OF_UHEAP_PAGES] = {0};

int get_page_index(uint32 va)
{
	return (va - (myEnv->env_rLimit + PAGE_SIZE)) / PAGE_SIZE;
}
void mark_pages_allocated(uint32 start_index, uint32 num_pages) {
    for (uint32 i = 0; i < num_pages; i++) {
    	page_marked[start_index + i] = 1;
    }
    virtual_addresses_pages_num[start_index] = num_pages;
}
void mark_pages_free(uint32 start_index, uint32 num_pages) {
    for (uint32 i = 0; i < num_pages; i++) {
    	page_marked[start_index + i] = 0;
    }
    virtual_addresses_pages_num[start_index] = 0;
}

uint32 FirstFit(uint32 num_pages){
    uint32 consecutive_free = 0;
    uint32 start_page = 0;
    for (uint32 page = 0; page < NUM_OF_UHEAP_PAGES; page++) {
        if (page_marked[page] == 0) {
            if (consecutive_free == 0) {
                start_page = page;
            }
            consecutive_free++;
            uint32 va = (myEnv->env_rLimit + PAGE_SIZE) + (start_page * PAGE_SIZE);
            uint32 end_va = va + (num_pages * PAGE_SIZE);
            if (consecutive_free >= num_pages && end_va <= USER_HEAP_MAX) {
                return va;
            }
        }
        else
            consecutive_free = 0;
    }
    return 0;
}
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	       	{
	       		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE)
	       		{
	       			return alloc_block_FF(size);
	       	    }
	    uint32 num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	    uint32 va = FirstFit(num_pages);

	    if (va == 0) return NULL;

	    sys_allocate_user_mem(va, size);
	    mark_pages_allocated(get_page_index(va), num_pages);
	    return (void*)va;
	       	}
	       	    return (void *) -1;

}
//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	uint32 va = (uint32)virtual_address;

	    if(va>=USER_HEAP_START  && va<myEnv->env_segBreak){
	        		free_block(virtual_address);
	        	}
	    else if(va>=myEnv->env_rLimit+PAGE_SIZE && va<USER_HEAP_MAX){
	        uint32 start_page = get_page_index(va);
	        uint32 num_pages = virtual_addresses_pages_num[start_page];
	        sys_free_user_mem(va, num_pages * PAGE_SIZE);
	        mark_pages_free(start_page, num_pages);
	    } else {
	        panic("invalid address");
	    }

}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
//	panic("smalloc() is not implemented yet...!!");

	uint32 num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	uint32 va = FirstFit(num_pages);

	if(va == 0)
		return NULL;

	int check = sys_createSharedObject(sharedVarName, size, isWritable, (void *)va);

	if(check == E_SHARED_MEM_EXISTS || check == E_NO_SHARE)
		return NULL;

	mark_pages_allocated(get_page_index(va), num_pages);
	slave_to_master[get_page_index(va)]=va;

	return (void *) va;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName) {
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");



	    int size = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);
	    if (size == E_SHARED_MEM_NOT_EXISTS) {

	        return NULL;
	    }
	    uint32 num_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	    uint32 va = FirstFit(num_pages);
	    if (va == 0) {

	        return NULL;
	    }

	    int ret = sys_getSharedObject(ownerEnvID, sharedVarName, (void*)va);
	    if (ret == E_SHARED_MEM_NOT_EXISTS) {

	        return NULL;
	    }

	    slave_to_master[get_page_index(va)]=ret;

	    mark_pages_allocated(get_page_index(va), num_pages);


	return (void*)va;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
//	panic("sfree() is not implemented yet...!!");
	uint32 Id = slave_to_master[get_page_index((uint32)virtual_address)] & 0x7FFFFFFF;
	sys_freeSharedObject(Id, virtual_address);
	uint32 va = (uint32)virtual_address;
	uint32 page_index = get_page_index(va);
	uint32 num_pages = virtual_addresses_pages_num[get_page_index(va)];
	mark_pages_free(page_index, num_pages);

}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
