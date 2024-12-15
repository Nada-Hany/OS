/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/mem/memory_manager.h>


//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//==================
// [1] MAIN HANDLER:
//==================
/*2022*/
uint32 last_eip = 0;
uint32 before_last_eip = 0;
uint32 last_fault_va = 0;
uint32 before_last_fault_va = 0;
int8 num_repeated_fault  = 0;

struct Env* last_faulted_env = NULL;
void fault_handler(struct Trapframe *tf)
{
	//cprintf("newtest\n\n");
	/******************************************************/
	// Read processor's CR2 register to find the faulting address
	uint32 fault_va = rcr2();
	//	cprintf("\n************Faulted VA = %x************\n", fault_va);
	//	print_trapframe(tf);
	/******************************************************/

	//If same fault va for 3 times, then panic
	//UPDATE: 3 FAULTS MUST come from the same environment (or the kernel)
	struct Env* cur_env = get_cpu_proc();
	if (last_fault_va == fault_va && last_faulted_env == cur_env)
	{
		num_repeated_fault++ ;
		if (num_repeated_fault == 3)
		{
			print_trapframe(tf);
			panic("Failed to handle fault! fault @ at va = %x from eip = %x causes va (%x) to be faulted for 3 successive times\n", before_last_fault_va, before_last_eip, fault_va);
		}
	}
	else
	{
		before_last_fault_va = last_fault_va;
		before_last_eip = last_eip;
		num_repeated_fault = 0;
	}
	last_eip = (uint32)tf->tf_eip;
	last_fault_va = fault_va ;
	last_faulted_env = cur_env;
	/******************************************************/
	//2017: Check stack overflow for Kernel
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	if (!userTrap)
	{
		struct cpu* c = mycpu();
		//cprintf("trap from KERNEL\n");
		if (cur_env && fault_va >= (uint32)cur_env->kstack && fault_va < (uint32)cur_env->kstack + PAGE_SIZE)
			panic("User Kernel Stack: overflow exception!");
		else if (fault_va >= (uint32)c->stack && fault_va < (uint32)c->stack + PAGE_SIZE)
			panic("Sched Kernel Stack of CPU #%d: overflow exception!", c - CPUS);
#if USE_KHEAP
		if (fault_va >= KERNEL_HEAP_MAX)
			panic("Kernel: heap overflow exception!");
#endif
	}
	//2017: Check stack underflow for User
	else
	{
		//cprintf("trap from USER\n");
		if (fault_va >= USTACKTOP && fault_va < USER_TOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	//cprintf("curenv = %x\n", curenv);
	struct Env* faulted_env = cur_env;
	if (faulted_env == NULL)
	{
		print_trapframe(tf);
		panic("faulted env == NULL!");
	}
	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ( (faulted_env->env_page_directory[PDX(fault_va)] & PERM_PRESENT) != PERM_PRESENT)
	{
		// we have a table fault =============================================================
		//		cprintf("[%s] user TABLE fault va %08x\n", curenv->prog_name, fault_va);
		//		print_trapframe(tf);

		faulted_env->tableFaultsCounter ++ ;

		table_fault_handler(faulted_env, fault_va);
	}
	else
	{
		if (userTrap)
		{
			/*============================================================================================*/
			//TODO: [PROJECT'24.MS2 - #08] [2] FAULT HANDLER I - Check for invalid pointers
			//(e.g. pointing to unmarked user heap page, kernel or wrong access rights),
			//your code is here
			if(fault_va >= USER_LIMIT)
			{
				//cprintf("in1\n");
				env_exit();
			}
			int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
			if ((perms & PERM_PRESENT) && !(perms & PERM_WRITEABLE))
			{
				    //cprintf("in2\n");
					env_exit();
			}
			if(!(perms & PERM_MARKED) && (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX))
			{
				env_exit();
			}
			/*============================================================================================*/
		}

		/*2022: Check if fault due to Access Rights */
		int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
		if (perms & PERM_PRESENT)
			panic("Page @va=%x is exist! page fault due to violation of ACCESS RIGHTS\n", fault_va) ;
		/*============================================================================================*/


		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter ++ ;

		//		cprintf("[%08s] user PAGE fault va %08x\n", curenv->prog_name, fault_va);
		//		cprintf("\nPage working set BEFORE fault handler...\n");
		//		env_page_ws_print(curenv);

		if(isBufferingEnabled())
		{
			__page_fault_handler_with_buffering(faulted_env, fault_va);
		}
		else
		{
			//page_fault_handler(faulted_env, fault_va);
			page_fault_handler(faulted_env, fault_va);
		}
		//		cprintf("\nPage working set AFTER fault handler...\n");
		//		env_page_ws_print(curenv);

	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/
}
//=========================
// [2] TABLE FAULT HANDLER:
//=========================
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}
//=========================
// [3] PAGE FAULT HANDLER:
//=========================
bool is_stack_or_heap_page(uint32 va)
{
	return ((va >= USER_HEAP_START && va < USER_HEAP_MAX) || (va >= USTACKBOTTOM  && va < USTACKTOP));
}
void free_frame_and_exit(struct Env * faulted_env, uint32 fault_va, struct FrameInfo *frame_info)
{
	free_frame(frame_info);
	unmap_frame(faulted_env->env_page_directory, fault_va);
    env_exit();
}
void replace(struct Env * faulted_env, uint32 fault_va, struct WorkingSetElement *victimWSElement)
{
    uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, victimWSElement->virtual_address);
    if (perms & PERM_MODIFIED)
    {
        uint32 *page_table_ptr;
        struct FrameInfo *ptr_frame_info = get_frame_info(faulted_env->env_page_directory, victimWSElement->virtual_address, &page_table_ptr);
        pf_update_env_page(faulted_env, victimWSElement->virtual_address, ptr_frame_info);
    }
    pages_alloc_in_WS_list[(victimWSElement->virtual_address - (faulted_env->env_rLimit + PAGE_SIZE)) / PAGE_SIZE] = NULL;
    pages_alloc_in_WS_list[(fault_va - (faulted_env->env_rLimit + PAGE_SIZE)) / PAGE_SIZE] = victimWSElement;
    if(victimWSElement == LIST_LAST(&(faulted_env->page_WS_list)))
    	faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
    else
        faulted_env->page_last_WS_element = LIST_NEXT(victimWSElement);
    uint32 *ptr_page_table;
    struct FrameInfo *ptr_new_frame = get_frame_info(faulted_env->env_page_directory, victimWSElement->virtual_address, &ptr_page_table);
    map_frame(faulted_env->env_page_directory, ptr_new_frame,  fault_va, PERM_USER | PERM_WRITEABLE);
    unmap_frame(faulted_env->env_page_directory, victimWSElement->virtual_address);
    pt_set_page_permissions(faulted_env->env_page_directory, victimWSElement->virtual_address, 0, PERM_USED|PERM_PRESENT|PERM_MARKED);
    victimWSElement->virtual_address = fault_va;
    victimWSElement->sweeps_counter = 0;
    int pagefile_exists = pf_read_env_page(faulted_env, (void*)victimWSElement->virtual_address);
    if (pagefile_exists == E_PAGE_NOT_EXIST_IN_PF && !(is_stack_or_heap_page(fault_va)))
    {
        env_exit();
    }

}
void page_fault_handler(struct Env * faulted_env, uint32 fault_va)
{

#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(faulted_env->page_WS_list));
#else
		int iWS =faulted_env->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(faulted_env);
#endif
	if(isPageReplacmentAlgorithmNchanceCLOCK())
	{
		if(wsSize < (faulted_env->page_WS_max_size))
		{
			//TODO: [PROJECT'24.MS2 - #09] [2] FAULT HANDLER I - Placement
			struct FrameInfo *ptr_new_frame;
			allocate_frame(&ptr_new_frame);
			map_frame(faulted_env->env_page_directory, ptr_new_frame,  fault_va, PERM_USER | PERM_WRITEABLE);
		    int pagefile_exists = pf_read_env_page(faulted_env, (void*)fault_va);
		    if (pagefile_exists == E_PAGE_NOT_EXIST_IN_PF && !(is_stack_or_heap_page(fault_va)))
		    {
		        free_frame_and_exit(faulted_env, fault_va, ptr_new_frame);

		    }
			struct WorkingSetElement* working_set_element = env_page_ws_list_create_element(faulted_env, fault_va);
			pages_alloc_in_WS_list[(fault_va-(faulted_env->env_rLimit+PAGE_SIZE))/PAGE_SIZE]=working_set_element;
			if(faulted_env->page_last_WS_element == NULL)
			{
				LIST_INSERT_TAIL(&(faulted_env->page_WS_list), working_set_element);
				if (LIST_SIZE(&(faulted_env->page_WS_list)) == faulted_env->page_WS_max_size)
				     faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
			}
			else
			   LIST_INSERT_BEFORE(&(faulted_env->page_WS_list), faulted_env->page_last_WS_element, working_set_element);
		}
		else
		{
			//TODO: [PROJECT'24.MS3] [2] FAULT HANDLER II - Replacement
			//env_page_ws_print(faulted_env);
			int N = page_WS_max_sweeps;
			if(N==0)
			{
				 replace(faulted_env, fault_va, faulted_env->page_last_WS_element);
				 return;
			}

			//////////////// GET VICTIM WS ELEMENT ////////////////

			int max_counter = -1;
			int i = -1, victim_indx = -1, lastelem_indx = -1, notmodified_indx = -1;
			struct WorkingSetElement *closest_notmodified_element = NULL, *wse;
			bool modified = (N < 0);
			LIST_FOREACH(wse, &(faulted_env->page_WS_list))
			{
				i++;
				if (wse == faulted_env->page_last_WS_element)
				    lastelem_indx = i;

				 uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, wse->virtual_address);

				 if (!(perms & PERM_MODIFIED) && (notmodified_indx == -1 || (lastelem_indx > -1 && notmodified_indx < lastelem_indx)))
				 {
				     closest_notmodified_element = wse;
				     notmodified_indx = i;
				 }

				  if (perms & PERM_USED)
				      continue;

				  int counter = wse->sweeps_counter;
				  if (modified && (perms & PERM_MODIFIED))
				      counter--;

				  if (counter > max_counter || (counter == max_counter && lastelem_indx > -1 && victim_indx < lastelem_indx))
				  {
				      max_counter = counter;
				      victimWSElement = wse;
				      victim_indx = i;
				  }
			  }
			  if (victimWSElement == NULL)
			  {
				  if(closest_notmodified_element && modified)
				  {
					  victimWSElement = closest_notmodified_element;
					  victim_indx = notmodified_indx;
				  }
				  else
				  {
					  victimWSElement = faulted_env->page_last_WS_element;
					  victim_indx = lastelem_indx;
				   }
				  max_counter=0;
				  if(!closest_notmodified_element && modified)
					  max_counter--;
			  }

			  //////////////// UPDATE SWEEPS COUNTER ////////////////

			  if (modified) N *= -1;
			  int diff = N - max_counter;
			  uint32 victim_perms = pt_get_page_permissions(faulted_env->env_page_directory, victimWSElement->virtual_address);
			  bool victim_used = (victim_perms & PERM_USED);
			  bool victim_modified = (victim_perms & PERM_MODIFIED);
			  i = -1;
			  bool complete_loop = (diff > 1 || victim_used);

			  LIST_FOREACH(wse, &(faulted_env->page_WS_list))
			  {
			      i++;
			      uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, wse->virtual_address);
			      bool used = (perms & PERM_USED);
			      bool within_range;

			      if (victim_indx == lastelem_indx)
			          within_range = (i == victim_indx);
			      else if (victim_indx > lastelem_indx)
			          within_range = (i >= lastelem_indx && i <= victim_indx);
			      else
			          within_range = (i >= lastelem_indx || i <= victim_indx);

			      if (used)
			      {
			    	  if(complete_loop || (!complete_loop && within_range) ){
			              pt_set_page_permissions(faulted_env->env_page_directory, wse->virtual_address, 0, PERM_USED);
			              wse->sweeps_counter = 0;
			    	  }
			          if (!complete_loop)
			              continue;
			      }

			      if (within_range)
			          wse->sweeps_counter += (used && !victim_used) ? diff - 1 : diff;
			      else
			          wse->sweeps_counter += (used && !victim_used) ? diff - 2 : diff - 1;
			  }

			  //////////////// REPLACEMENT ////////////////

			  replace(faulted_env, fault_va, victimWSElement);
		}
	}
}
void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	//[PROJECT] PAGE FAULT HANDLER WITH BUFFERING
	// your code is here, remove the panic and write your code
	panic("__page_fault_handler_with_buffering() is not implemented yet...!!");
}

