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
	memset(virtual_addresses, 0, sizeof(virtual_addresses));
	if(segBreak>rLimit)
		panic("initial size exceeds the given limit");
	uint32 va = start;
	while(va<segBreak){
		struct FrameInfo *ptr_frame_info ;
		int ret = allocate_frame(&ptr_frame_info);
		if (ret == E_NO_MEM){
			panic("no enough memory to allocate a frame\n");
		}
		ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
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
//	cprintf("sbrk called\n");
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
				if (ret == E_NO_MEM){
					panic("no enough memory to allocate a frame\n");
				}
				ret = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
				if (ret == E_NO_MEM){
					free_frame(ptr_frame_info) ;
					panic("No enough memory for page table!\n");
				}
				va = va + PAGE_SIZE;
		}
	}
//	cprintf("sbrk finished\n");
	return (void*)previous_segBreak;
	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

int num_of_unmapped_pages(uint32 start_va){
	int num = 0;
	while(start_va<KERNEL_HEAP_MAX){
		if(kheap_physical_address(start_va) != 0){
			start_va = start_va + PAGE_SIZE;
			continue;
		}
		num = num + 1;
		start_va = start_va + PAGE_SIZE;
	}
	return num;
}
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	if(isKHeapPlacementStrategyFIRSTFIT()){
		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
			return alloc_block_FF(size);
		}else{
			uint32 va = rLimit + PAGE_SIZE;

			int free_frames=LIST_SIZE(&MemFrameLists.free_frame_list);
			int unmapped_pages=num_of_unmapped_pages(va);
			if(free_frames*PAGE_SIZE<size || unmapped_pages*PAGE_SIZE<size)
				return NULL;

			int pages_to_alloc = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
			//cprintf ("pages_to_alloc %d\n",pages_to_alloc);
			int found_start=0;
			uint32 actual_start=0;

			while(pages_to_alloc){
				uint32 to_map_page = va;
				if(kheap_physical_address(to_map_page) != 0){
					va = va + PAGE_SIZE;
					continue;
				}
				if(PAGE_SIZE>size)
					to_map_page = to_map_page + size;

				if(!found_start){
					actual_start=to_map_page;
					found_start=1;
				}

				struct FrameInfo *ptr_frame_info ;
				allocate_frame(&ptr_frame_info);

				map_frame(ptr_page_directory, ptr_frame_info, to_map_page, PERM_WRITEABLE);

				va = va + PAGE_SIZE;
				pages_to_alloc=pages_to_alloc-1;
			}
			virtual_addresses_pages_num[actual_start>>12]=(size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
			//cprintf(" va : %d,    va of allocated in alloc %d\n",actual_start,virtual_addresses_pages_num[va>>12]);

			return (void *)actual_start;
		}
	}
	return (void *) -1;
}

void kfree(void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	//cprintf("va ptr %d\n", virtual_address);
	uint32 va = (uint32) virtual_address;
	//cprintf("va after cast %d\n", va);
	if (va < KERNEL_HEAP_START || va > KERNEL_HEAP_MAX) {
		panic("invalid virtual address");
	}

	if (va < segBreak && va < rLimit) {
	//	cprintf("to block");
		free_block(virtual_address);
	} else if (va > rLimit + 4) {

	//	cprintf("to page\n");

		uint32 num_of_pages = virtual_addresses_pages_num[va >> 12];
//		cprintf("num_of_pages%d \n ",virtual_addresses_pages_num[va>>12]);
		//cprintf(" va : %d,    va of allocated in alloc %d\n", va,virtual_addresses_pages_num[va >> 12]);
		uint32 actual_start_free = va;
		//cprintf("num of free before %d\n",LIST_SIZE(&MemFrameLists.free_frame_list));
		while (num_of_pages) {
			uint32* ptr_page_table = NULL;
			struct FrameInfo *ptr_frame = get_frame_info(ptr_page_directory, va,
					&ptr_page_table);

			free_frame(ptr_frame);
			unmap_frame(ptr_page_directory, va);


			va = va + PAGE_SIZE;
			//cprintf("free block %d\n", num_of_pages);
			num_of_pages--;

		}
		virtual_addresses_pages_num[actual_start_free >> 12] = 0;
		//cprintf("actual add %d\n", actual_start_free);

		//cprintf("num of free after %d\n",LIST_SIZE(&MemFrameLists.free_frame_list));
	} else {
		panic("invalid virtual address");
	}
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
//	cprintf("va:0x%x\n",virtual_address);
	//getting the page table start address
//	uint32 page_directory_entry = ptr_page_directory[PDX(virtual_address)];
//	uint32 page_directory_frame_address = EXTRACT_ADDRESS(page_directory_entry);
	uint32 *page_table_ptr;
//	get_page_table(ptr_page_directory,virtual_address,page_table_ptr);
	if(get_page_table(ptr_page_directory,virtual_address,&page_table_ptr) == TABLE_NOT_EXIST){
		return 0;
	}
	//getting the page address
	uint32 page_table_entry_address  = page_table_ptr[PTX(virtual_address)];
	uint32 page_address = EXTRACT_ADDRESS(page_table_entry_address);
	if(page_address == 0){
		return 0;
	}
	//adding offset and return
	uint32 offset = PGOFF(virtual_address);
	uint32 physical_address = page_address + offset;
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
	uint32 offset = PGOFF(physical_address);
	return virtual_addresses[physical_address>>12]+offset;
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

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



void reallocate_loaded_frames(struct FrameInfo **frames_ptr){


}

void* getunlocatedFrames(int framesNumber){

}

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code

	if (virtual_address == NULL && new_size == 0)
			return NULL;

	if (new_size == 0){
		kfree(virtual_address);
		return NULL;
	}

	if(virtual_address == NULL)
		return kmalloc(new_size);


	uint32 va = (uint32) virtual_address;
	uint32 old_pages = virtual_addresses_pages_num[va >> 12];
	uint32 old_size = old_pages * PAGE_SIZE;
	uint32 new_pages = (new_size/PAGE_SIZE) + ((new_size%PAGE_SIZE!=0)?1:0);



	// lw kan f el block allocation w hay-switch l page hageb el old data ezay
	// lw address block haygy mn awel el header wla awelo

	// check if process will reallocate to a block or not
	bool to_block_allocation = 0;
	if(new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		to_block_allocation = 1;

	void* new_address = NULL;

	// was in block allocation
	if (va < segBreak && va < rLimit ) {
		if(to_block_allocation)
			return realloc_block_FF(virtual_address, new_size);
		else{
			uint32 old_block_size = get_block_size(virtual_address);
			new_address = kmalloc(new_size);
			memcpy(new_address , virtual_address, old_block_size);
			free_block(virtual_address);
			return new_address ;
		}

	}
	// was in page allocation
	else if (va > rLimit + 4) {
		char* new_ptr = (char*) new_address;
		char* old_ptr = (char *)virtual_address;

		if(to_block_allocation){
			uint32 old_block_size = get_block_size(virtual_address);
			char tmp[old_block_size];
			for (uint32 i = 0; i < old_block_size; i++)
				 tmp[i] = old_ptr[i];
			kfree(virtual_address);
			new_address = (void *) alloc_block_FF(new_size);
			memcpy(new_address, (void*)tmp, old_size);
			return new_address;

		}else{
			// reallocating in page section
			// to bigger size
			if(new_pages >= old_pages){

			}
			// to smaller size
			else{;
				kfree(virtual_address);
				new_address = kmalloc(new_size);
				memcpy(new_address, virtual_address, new_size);
				return new_address;
			}
		}


	} else {
		panic("invalid virtual address");
	}

	return NULL;
//	panic("krealloc() is not implemented yet...!!");
}
