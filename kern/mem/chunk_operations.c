/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/proc/user_environment.h>
#include "kheap.h"
#include "memory_manager.h"
#include <inc/queue.h>

//extern void inctst();

/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va on the given page_directory
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, cut-paste the number of pages and return 0
//	ALL 12 permission bits of the destination should be TYPICAL to those of the source
//	The given addresses may be not aligned on 4 KB
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 num_of_pages)
{
	//[PROJECT] [CHUNK OPERATIONS] cut_paste_pages
	// Write your code here, remove the panic and write your code
	panic("cut_paste_pages() is not implemented yet...!!");
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	If ANY of the destination pages exists with READ ONLY permission, deny the entire process and return -1.
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages doesn't exist, create it with the following permissions then copy.
//	Otherwise, just copy!
//		1. WRITABLE permission
//		2. USER/SUPERVISOR permission must be SAME as the one of the source
//	The given range(s) may be not aligned on 4 KB
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va, uint32 size)
{
	//[PROJECT] [CHUNK OPERATIONS] copy_paste_chunk
	// Write your code here, remove the //panic and write your code
	panic("copy_paste_chunk() is not implemented yet...!!");
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	It should set the permissions of the second range by the given perms
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, share the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	The given range(s) may be not aligned on 4 KB
int share_chunk(uint32* page_directory, uint32 source_va,uint32 dest_va, uint32 size, uint32 perms)
{
	//[PROJECT] [CHUNK OPERATIONS] share_chunk
	// Write your code here, remove the //panic and write your code
	panic("share_chunk() is not implemented yet...!!");
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate the given virtual range [<va>, <va> + <size>) in the given address space  <page_directory> with the given permissions <perms>.
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, allocate the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	Allocation should be aligned on page boundary. However, the given range may be not aligned.
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms)
{
	//[PROJECT] [CHUNK OPERATIONS] allocate_chunk
	// Write your code here, remove the //panic and write your code
	panic("allocate_chunk() is not implemented yet...!!");
}

//=====================================
// 5) CALCULATE ALLOCATED SPACE IN RAM:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva, uint32 *num_tables, uint32 *num_pages)
{
	//[PROJECT] [CHUNK OPERATIONS] calculate_allocated_space
	// Write your code here, remove the panic and write your code
	panic("calculate_allocated_space() is not implemented yet...!!");
}

//=====================================
// 6) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
//This function should calculate the required number of pages for allocating and mapping the given range [start va, start va + size) (either for the pages themselves or for the page tables required for mapping)
//	Pages and/or page tables that are already exist in the range SHOULD NOT be counted.
//	The given range(s) may be not aligned on 4 KB
uint32 calculate_required_frames(uint32* page_directory, uint32 sva, uint32 size)
{
	//[PROJECT] [CHUNK OPERATIONS] calculate_required_frames
	// Write your code here, remove the panic and write your code
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================

//=====================================
/* DYNAMIC ALLOCATOR SYSTEM CALLS */
//=====================================
void* sys_sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the current user program to increase the size of its heap
	 * 				by the given number of pages. You should allocate NOTHING,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) As in real OS, allocate pages lazily. While sbrk moves the segment break, pages are not allocated
	 * 		until the user program actually tries to access data in its heap (i.e. will be allocated via the fault handler).
	 * 	2) Allocating additional pages for a process� heap will fail if, for example, the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sys_sbrk fails, the net effect should
	 * 		be that sys_sbrk returns (void*) -1 and that the segment break and the process heap are unaffected.
	 * 		You might have to undo any operations you have done so far in this case.
	 */

	//TODO: [PROJECT'24.MS2 - #11] [3] USER HEAP - sys_sbrk
	/*====================================*/
	/*Remove this line before start coding*/
	//return (void*)-1 ;
	/*====================================*/
//	cprintf("in user sbrk\n");
	struct Env* env = get_cpu_proc(); //the current running Environment to adjust its break limit
	uint32 previous_segBreak = env->env_segBreak;
	if(numOfPages>0){

			uint32 increase = numOfPages * PAGE_SIZE;
			if(increase + env->env_segBreak > env->env_rLimit){
				return (void*)-1;
			}
			//move sbrk
			env->env_segBreak+=increase;
			//mark pages
			uint32 *page_table_ptr;
			int num_of_pages = numOfPages;
			uint32 virtual_address = previous_segBreak;
			while(num_of_pages--)
			{
				if(get_page_table(env->env_page_directory, virtual_address, &page_table_ptr) == TABLE_NOT_EXIST)
				{
					//cprintf("creating new pt\n");
					page_table_ptr = create_page_table(env->env_page_directory, virtual_address);
				}
				pt_set_page_permissions(env->env_page_directory, virtual_address, PERM_MARKED, 0);
				//int perms = pt_get_page_permissions(e->env_page_directory, virtual_address);
				//cprintf("perms : %d\n", perms);
				virtual_address += PAGE_SIZE;
			}

	}
	//cprintf("\nsbrk_user will return here with num of page = %d \n" ,numOfPages);
	return (void *)previous_segBreak;
}

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	/*====================================*/
	/*Remove this line before start coding*/
//	inctst();
//	return;
	/*====================================*/

	//TODO: [PROJECT'24.MS2 - #13] [3] USER HEAP [KERNEL SIDE] - allocate_user_mem()
	// Write your code here, remove the panic and write your code
	//panic("allocate_user_mem() is not implemented yet...!!");
	//cprintf("in allocate_user_mem\n");
	int num_of_pages = (size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	//cprintf("num of pages : %d\n", num_of_pages);
	uint32 *page_table_ptr;
	while(num_of_pages--)
	{
		if(get_page_table(e->env_page_directory, virtual_address, &page_table_ptr) == TABLE_NOT_EXIST)
		{
			//cprintf("creating new pt\n");
			page_table_ptr = create_page_table(e->env_page_directory, virtual_address);
		}
		pt_set_page_permissions(e->env_page_directory, virtual_address, PERM_MARKED, 0);
        //int perms = pt_get_page_permissions(e->env_page_directory, virtual_address);
        //cprintf("perms : %d\n", perms);
		virtual_address += PAGE_SIZE;
	}
}

//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size)
{
	/*====================================*/
	/*Remove this line before start coding*/
//	inctst();
//	return;
	/*====================================*/

	//TODO: [PROJECT'24.MS2 - #15] [3] USER HEAP [KERNEL SIDE] - free_user_mem
	// Write your code here, remove the panic and write your code
	//panic("free_user_mem() is not implemented yet...!!");
	uint32 iterator_va = virtual_address;
	int num_of_pages=(size/PAGE_SIZE) + ((size%PAGE_SIZE!=0)?1:0);
	while(num_of_pages--){
		uint32* ptr_page_table = NULL;

		struct FrameInfo *ptr_frame = get_frame_info(e->env_page_directory, iterator_va,
				&ptr_page_table);

		if(ptr_frame!=0){
			free_frame(ptr_frame);

			struct WorkingSetElement *wse = pages_alloc_in_WS_list[(iterator_va-(e->env_rLimit+PAGE_SIZE))/PAGE_SIZE];

			unmap_frame(e->env_page_directory, wse->virtual_address);

			if(LIST_SIZE(&(e->page_WS_list)) == 1)
				e->page_last_WS_element = NULL;
			else if (e->page_last_WS_element == wse)
			{
				if(wse == LIST_LAST(&(e->page_WS_list)))
					e->page_last_WS_element = LIST_FIRST(&(e->page_WS_list));
				else
					e->page_last_WS_element  = LIST_NEXT(wse);
			}
			LIST_REMOVE(&(e->page_WS_list), wse);

			kfree(wse);

			pages_alloc_in_WS_list[(iterator_va-(e->env_rLimit+PAGE_SIZE))/PAGE_SIZE]=NULL;
		}

		pt_set_page_permissions(e->env_page_directory, iterator_va,0,PERM_MARKED);
		pt_set_page_permissions(e->env_page_directory, iterator_va,0,PERM_PRESENT);


		pf_remove_env_page(e,iterator_va);
		iterator_va+=PAGE_SIZE;

	}


	//TODO: [PROJECT'24.MS2 - BONUS#3] [3] USER HEAP [KERNEL SIDE] - O(1) free_user_mem
}

//=====================================
// 2) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address, uint32 size)
{
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	//[PROJECT] [USER HEAP - KERNEL SIDE] move_user_mem
	//your code is here, remove the panic and write your code
	panic("move_user_mem() is not implemented yet...!!");
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

