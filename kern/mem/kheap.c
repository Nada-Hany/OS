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

	uint32 start_page_alloc_va=rLimit+PAGE_SIZE;
	uint32 num_of_free_pages=(KERNEL_HEAP_MAX-start_page_alloc_va)/PAGE_SIZE +
			((KERNEL_HEAP_MAX-start_page_alloc_va)%PAGE_SIZE!=0?1:0);
	free_consecutive_pages[0]=(struct FreePage){start_page_alloc_va,num_of_free_pages};

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
	for(int i=0;i<NUM_OF_UHEAP_PAGES;i++)
		pages_alloc_in_WS_list[i]=NULL;
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
//	cprintf("kheap_sbrk called\n");
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
//		uint32 daEnd=(uint32)segBreak-sizeof(uint32);
//		uint32 *endptr=(uint32 *)daEnd;
//		cprintf("putting end block\n");
//		*endptr=1;
//		cprintf("putting end block 2\n");
//		set_block_data((void*)previous_segBreak, segBreak-previous_segBreak, 1);
//		cprintf("setted block data\n");
//		free_block((void*)previous_segBreak);
//		cprintf("freed block block\n");
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

uint32 get_page_index(uint32 va)
{
	return (va - (rLimit + PAGE_SIZE)) / PAGE_SIZE;
}

int num_of_unmapped_pages(uint32 start_va){
	int num = 0;
	while(start_va<KERNEL_HEAP_MAX && kheap_physical_address(start_va) == 0){
		num = num + 1;
		start_va = start_va + PAGE_SIZE;
	}
	return num;
}
void allocate_pages(uint32 start_va,uint32 size){
	int num_of_pages=(size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	while(num_of_pages){
		uint32 to_map_page=start_va;
		if(num_of_pages==1)
			to_map_page=to_map_page+(size%PAGE_SIZE);

		struct FrameInfo *ptr_frame_info ;
		allocate_frame(&ptr_frame_info);

		map_frame(ptr_page_directory, ptr_frame_info, to_map_page, PERM_WRITEABLE);
		num_of_pages--;
		start_va=start_va+PAGE_SIZE;
	}
}

void remove_from_free_consecutive_pages(int start){
	for(int j=start;j<(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;j++){
		if(free_consecutive_pages[j+1].start_va==0){
			free_consecutive_pages[j].start_va=0;
			free_consecutive_pages[j].num_of_free_pages=0;
			break;
		}

		struct FreePage temp=free_consecutive_pages[j+1];
		free_consecutive_pages[j+1]=free_consecutive_pages[j];
		free_consecutive_pages[j]=temp;
	}
}

void insertion_sort(int ind){
	for(int i=ind;i>0;i--){
		if(free_consecutive_pages[i].start_va>free_consecutive_pages[i-1].start_va)
			break;
		struct FreePage temp=free_consecutive_pages[i-1];
		free_consecutive_pages[i-1]=free_consecutive_pages[i];
		free_consecutive_pages[i]=temp;

	}
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
//	kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	if(isKHeapPlacementStrategyFIRSTFIT()){
		if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
//			cprintf("from kmalloc\n");
			return alloc_block_FF(size);
		}else{
			int free_frames=LIST_SIZE(&MemFrameLists.free_frame_list);

			uint32 actual_start=0;

			for(int i=0;i<(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;i++){
				if(free_consecutive_pages[i].start_va==0)
					break;
				if(free_consecutive_pages[i].num_of_free_pages*PAGE_SIZE>=size && free_frames*PAGE_SIZE>=size){
					actual_start=free_consecutive_pages[i].start_va;
					free_consecutive_pages[i].num_of_free_pages-=((size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0));
					free_consecutive_pages[i].start_va+=(((size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0))*PAGE_SIZE);
					if(free_consecutive_pages[i].num_of_free_pages<=0){
						remove_from_free_consecutive_pages(i);

					}
					break;
				}
			}
			if(actual_start==0)
				return NULL;

			allocate_pages(actual_start,size);
			virtual_addresses_pages_num[get_page_index(actual_start)]=(size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
			//cprintf(" va : %d,    va of allocated in alloc %d\n",actual_start,virtual_addresses_pages_num[va>>12]);

			return (void *)actual_start;
		}
	}
	return (void *) -1;
}

void add_to_free_pages(uint32 va,uint32 num_of_pages){
	int merge_after_ind=-1;
	int merge_before_ind=-1;

	for(int i=0;i<(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;i++){
		if(free_consecutive_pages[i].start_va==0)
			break;

		if(va+num_of_pages*PAGE_SIZE==free_consecutive_pages[i].start_va){
			merge_after_ind=i;
			num_of_pages+=free_consecutive_pages[i].num_of_free_pages;
		}

		if(free_consecutive_pages[i].start_va+free_consecutive_pages[i].num_of_free_pages*PAGE_SIZE==va)
			merge_before_ind=i;

	}
	if(merge_after_ind!=-1){
		remove_from_free_consecutive_pages(merge_after_ind);
	}
	if(merge_before_ind!=-1)
		free_consecutive_pages[merge_before_ind].num_of_free_pages+=num_of_pages;
	else{

		int last_ind=0;
		for(int i=0;i<(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;i++){
			if(free_consecutive_pages[i].start_va==0){
				free_consecutive_pages[i]=(struct FreePage) {va,num_of_pages};
				last_ind=i;
				break;
			}
		}

		insertion_sort(last_ind);

	}

}
void kfree(void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	uint32 va = (uint32) virtual_address;
	if (va < KERNEL_HEAP_START || va > KERNEL_HEAP_MAX) {
		panic("invalid virtual address");
	}

	if (va < segBreak && va < rLimit) {
		free_block(virtual_address);
	} else if (va > rLimit + 4) {


		uint32 num_of_pages = virtual_addresses_pages_num[get_page_index(va)];

		uint32 actual_start_free = va;

		while (num_of_pages) {
			uint32* ptr_page_table = NULL;
			struct FrameInfo *ptr_frame = get_frame_info(ptr_page_directory, va,
					&ptr_page_table);

			free_frame(ptr_frame);
			unmap_frame(ptr_page_directory, va);


			va = va + PAGE_SIZE;

			num_of_pages--;

		}
		add_to_free_pages(actual_start_free,virtual_addresses_pages_num[get_page_index(actual_start_free)]);
		virtual_addresses_pages_num[get_page_index(actual_start_free)] = 0;



	} else {
		panic("invalid virtual address");
	}
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{

	uint32 *page_table_ptr;

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
	uint32 va = virtual_addresses[physical_address>>12] & ~0xFFF;
	if(va!=0){
		va+=offset;
	}
	return va;
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


void * getunlocatedFrames(int framesNumber){
	int count = 0;
	uint32 va = KERNEL_HEAP_START;
	for(int i=0; i<(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE; i++){
		uint32 *ptr_page_table;
		get_page_table(ptr_page_directory, va, &ptr_page_table);
		uint32 page_table_entry = ptr_page_table[PTX(va)];

		if((page_table_entry & PERM_PRESENT) == PERM_PRESENT)
			count = 0;
		else
			count++;

		if(count == framesNumber)
			return (void *) va - (PAGE_SIZE * framesNumber);

		va += PAGE_SIZE;
	}
	return NULL;
}

int canAlocateAfter(uint32 va_start, int required_pages){
	int count = 0;
	for(int i=va_start; i<(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE; i++){
		uint32 *ptr_page_table;
		if(get_page_table(ptr_page_directory, va_start, &ptr_page_table) == 0)
			return 0;
		uint32 page_table_entry = ptr_page_table[PTX(va_start)];

		if((page_table_entry & PERM_PRESENT) == PERM_PRESENT)
			return 0;
		else
			count++;

		if(count == required_pages)
			return 1;

		va_start += PAGE_SIZE;
	}
	return 0;
}

int getFreeFrames(){

	int free_frames_number = 0;
	bool lock_already_held = holding_spinlock(&MemFrameLists.mfllock);
	if (!lock_already_held)
		acquire_spinlock(&MemFrameLists.mfllock);

	free_frames_number = LIST_SIZE(&MemFrameLists.free_frame_list);

	if (!lock_already_held)
		release_spinlock(&MemFrameLists.mfllock);

	return free_frames_number;
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

	// check if process will reallocate as a block or not
	bool to_block_allocation = 0;
	if(new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		to_block_allocation = 1;

	void* new_address = NULL;

	// was in block allocation
	if (va < segBreak && va < rLimit ) {
		if(to_block_allocation)
			return realloc_block_FF(virtual_address, new_size);
		else{
			uint32 old_block_size = get_block_size(virtual_address) - 8;

			new_address = kmalloc(new_size);
			if(new_address == NULL)
				return NULL;
			//copy data without considering footer data
			memcpy(new_address , virtual_address, old_block_size - 4);
			free_block(virtual_address);
			return new_address ;
		}
	}
	// was in page allocation
	else if (va > rLimit + 4) {

		// switch to block allocation section
		if(to_block_allocation){
			uint32 old_block_size = get_block_size(virtual_address) - 8;
			new_address = (void *) alloc_block_FF(new_size);
			memcpy(new_address, virtual_address, old_block_size - 4);
			kfree(virtual_address);
			return new_address;

		}else{
			// reallocating in page section
			// to bigger size
			if(new_pages >= old_pages){
				int neededPages = new_pages - old_pages;
				uint32 start_add = va + (old_pages*PAGE_SIZE);
				// if there's enough space after old pages to be allocated
				if(canAlocateAfter(start_add, neededPages)){
					for(int i=0; i<neededPages; i++){
						struct FrameInfo *ptr_frame_info ;
						allocate_frame(&ptr_frame_info);
						map_frame(ptr_page_directory, ptr_frame_info, start_add, PERM_WRITEABLE);
						start_add += PAGE_SIZE;
					}
					return virtual_address;

				}else{
					new_address = kmalloc(new_size);
					if(new_address == NULL)
						return NULL;
					memcpy(new_address, virtual_address, old_size);
					kfree(virtual_address);
					return new_address;
				}
			}
			// to smaller size
			else{
				new_address = kmalloc(new_size);
				memcpy(new_address, virtual_address, new_size);
				kfree(virtual_address);
				return new_address;
			}
		}

	} else
		panic("invalid virtual address");

	return NULL;
//	panic("krealloc() is not implemented yet...!!");
}
