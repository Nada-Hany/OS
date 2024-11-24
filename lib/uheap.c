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
//	cprintf("sbrk_user\n");
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
uint32 virtual_addresses_pages_num [NUM_OF_UHEAP_PAGES];
bool page_marked[NUM_OF_UHEAP_PAGES] = {0};

int get_page_index(uint32 va)
{
	return (va - (myEnv->env_rLimit + PAGE_SIZE)) / PAGE_SIZE;
}

void mark_pages(uint32 start_va, uint32 size)
{
	//cprintf("in mark_pages\n");
	int num_of_pages = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	while(num_of_pages--)
	{
		uint32 page_index = get_page_index(start_va);
		page_marked[page_index] = 1;
		start_va += PAGE_SIZE;
	}
}
void unmark_pages(uint32 start_va, uint32 size)
{
	//cprintf("in mark_pages\n");
	int num_of_pages = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	while(num_of_pages--)
	{
		uint32 page_index = get_page_index(start_va);
		page_marked[page_index] = 0;
		start_va += PAGE_SIZE;
	}
}

int num_of_unmapped_pages(uint32 start_va)
{
	//cprintf("in num_of_unmapped_pages\n");
	int num = 0;
	uint32 page_index = get_page_index(start_va);
	//cprintf("%d\n", page_index);
	while(start_va < USER_HEAP_MAX && !(page_marked[page_index]))
	{
		num = num + 1;
		start_va = start_va + PAGE_SIZE;
		page_index = get_page_index(start_va);
	}
	return num;
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
		//cprintf("in malloc\n");
		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
//			cprintf("block allocator\n");
			return alloc_block_FF(size);
	    }
		else
		{
			uint32 start_va = myEnv->env_rLimit + PAGE_SIZE;
			uint32 final_va = 0;
			while(start_va < USER_HEAP_MAX)
			{
				uint32 page_index = get_page_index(start_va);
				if(page_marked[page_index])
				{
					start_va += PAGE_SIZE;
					continue;
				}

				int consecutive_free_pages = num_of_unmapped_pages(start_va);
				if(consecutive_free_pages*PAGE_SIZE >= size)
				{
					//cprintf("found\n");
					//cprintf("%d\n", consecutive_free_pages);
					//cprintf("%d\n", ((size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0)));
					final_va = start_va;
					break;
				}

				start_va += (consecutive_free_pages*PAGE_SIZE);
			}
			if(final_va==0)
				return NULL;

			mark_pages(final_va, size);
			sys_allocate_user_mem(final_va, size);
			//cprintf("after sys_alloc\n");
			virtual_addresses_pages_num[get_page_index(final_va)]=(size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
			return (void *) final_va;
		}
	}
	return (void *) -1;
}

uint32 FirstFit(uint32 start_va,uint32 size) {
	/*
	 * this function apply FirstFit strategy for user virtual memory
	 * Parameters:
	 * 				start_va -> start address of search
	 * 				size -> size in bytes not page or frame number
	 * return:
	 * 			if success ->start virtual address of allocation (uint32)
	 * 			if no pages-> return 0
	 * */

	//number of required pages

	cprintf("in ff func \n");
	uint32 num_of_pages=ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
//	uint32 num_of_pages = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	uint32 final_va = 0;
	while (start_va < USER_HEAP_MAX) {
		uint32 page_index = get_page_index(start_va);
//		cprintf("before checking marked pages\n");
		// If the current page is allocated, skip it
		if (page_marked[page_index]) {
//			cprintf("page marked");
			start_va += PAGE_SIZE;
			continue;
		}

		int consecutive_free_pages = num_of_unmapped_pages(start_va);
		if (consecutive_free_pages >= num_of_pages) {
			//cprintf("found\n");
			//cprintf("%d\n", consecutive_free_pages);
			//cprintf("%d\n", ((size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0)));
			final_va = start_va;
//			cprintf("final va in ff func -> %x \n", final_va);
			break;
		}
//		cprintf("in loop start va = %x \n ", start_va);
		// go to next block of free pages
		start_va += (consecutive_free_pages * PAGE_SIZE);
	}

	return final_va;
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
	}else if(va>=myEnv->env_rLimit+PAGE_SIZE && va<USER_HEAP_MAX){
		int num_of_pages = virtual_addresses_pages_num[get_page_index(va)];
		unmark_pages(va,num_of_pages*PAGE_SIZE);
		sys_free_user_mem(va,virtual_addresses_pages_num[get_page_index(va)]*PAGE_SIZE);
		virtual_addresses_pages_num[get_page_index(va)]=0;
	}else{
		panic("invalid address\n");
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

	cprintf("in smalloc\n");
	cprintf("frames in smalloc = %d\n", sys_calculate_free_frames());
//	cprintf(sharedVarName);
//	cprintf("\n");
	uint32 va = FirstFit(myEnv->env_rLimit + PAGE_SIZE, size);
//	cprintf("va in smalloc -> %x \n", va);
	if(va == 0)
		return NULL;

	int check = sys_createSharedObject(sharedVarName, size, isWritable, (void *)va);
	cprintf("frames in smalloc after create share object= %d\n", sys_calculate_free_frames());
//	cprintf("after sys create obj, check = %d \n", check);
	if(check == E_SHARED_MEM_EXISTS || check == E_NO_SHARE)
		return NULL;

	int numberOfFrames = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	// marking allocated pages as marked to be skipped
	uint32 tmp = va;
	while(numberOfFrames){
		int ind = get_page_index(tmp);
		page_marked[ind] = 1;
		tmp += PAGE_SIZE;
		numberOfFrames--;
	}
	cprintf("frames in smalloc after allocating its frame = %d\n", sys_calculate_free_frames());
	cprintf("end of smalloc\n");
	return (void *) va;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName) {
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	/*int size = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);
	// check if shared obj exit
	if (size == E_SHARED_MEM_NOT_EXISTS) {
		return NULL;
	}
	cprintf("in sget \n");
	uint32 va=FirstFit(myEnv->env_rLimit + PAGE_SIZE,size);

	if (va == 0)
		return NULL;
	uint32 ret = sys_getSharedObject(ownerEnvID, sharedVarName, (void*)va);
	if (ret == E_SHARED_MEM_NOT_EXISTS) {
		return NULL; // Failed to map the shared object
	}*/
	cprintf("sget: Starting retrieval for shared variable %s from owner %d\n", sharedVarName, ownerEnvID);

	    int size = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);
	    if (size == E_SHARED_MEM_NOT_EXISTS) {
	        cprintf("sget: Shared variable %s does not exist in environment %d\n", sharedVarName, ownerEnvID);
	        return NULL;
	    }

	    uint32 va = FirstFit(myEnv->env_rLimit + PAGE_SIZE, size);
	    if (va == 0) {
	        cprintf("sget: No suitable virtual address found for size %d\n", size);
	        return NULL;
	    }

	    int ret = sys_getSharedObject(ownerEnvID, sharedVarName, (void*)va);
	    if (ret == E_SHARED_MEM_NOT_EXISTS) {
	        cprintf("sget: Failed to map shared variable %s\n", sharedVarName);
	        return NULL;
	    }
	int numberOfPages = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);

	uint32 tmp = va;
	while(numberOfPages){
		int ind = get_page_index(tmp);
		page_marked[ind] = 1;
		tmp += PAGE_SIZE;
		numberOfPages--;
	}
//	cprintf("end of sget \n");
    cprintf("sget: Successfully retrieved and mapped shared variable %s\n", sharedVarName);
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
	//do i need to remove offset from the virtual address ????
	int32 Id = (uint32)virtual_address & 0x7FFFFFFF;
	sys_freeSharedObject(Id, virtual_address);
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
