#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	start = (uint32) daStart;
	segBreak = (uint32) daStart + initSizeToAllocate;
	rLimit = (uint32) daLimit;
	if(segBreak>rLimit)
		panic("initial size exceeds the given limit");
	uint32 va = start;
	while(va<segBreak){
		struct FrameInfo *ptr_frame_info ;
		int ret = allocate_frame(&ptr_frame_info);
		if (ret == E_NO_MEM){
			panic("no enough memory to allocate a frame\n");
		}
		ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_PRESENT);
		if (ret == E_NO_MEM){
			free_frame(ptr_frame_info) ;
			panic("No enough memory for page table!\n");
		}
		va = va + PAGE_SIZE;
	}
	initialize_dynamic_allocator(daStart,initSizeToAllocate);
	return 0;
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");
}

void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */
	uint32 previous_segBreak = segBreak;
	if(numOfPages>0){
		int free_frame_list_size = LIST_SIZE(&MemFrameLists.free_frame_list);
		uint32 increase = numOfPages * PAGE_SIZE;
		if(increase + segBreak > rLimit || free_frame_list_size < numOfPages){
			return (void*)-1;
		}
		segBreak+=increase;
		uint32 va = previous_segBreak;
		while(va<segBreak){
				struct FrameInfo *ptr_frame_info ;
				int ret = allocate_frame(&ptr_frame_info);
				ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_PRESENT);
				va = va + PAGE_SIZE;
		}
	}
	return (void*)previous_segBreak;
	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//frame address not frame number
	//pt frame number
	uint32 page_directory_entry = ptr_page_directory[PDX(virtual_address)];
	uint32 page_directory_frame_number = page_directory_entry >> 12;
//	uint32 page_table_address = page_directory_frame_number * PAGE_SIZE ;
	uint32 page_table_entry_address  = page_directory_frame_number + PTX(virtual_address) * 32;
	uint32 offset = PGOFF(virtual_address);
	uint32 physical_address = page_table_entry_address + offset;
	return physical_address;
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
