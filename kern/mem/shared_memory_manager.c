#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

	cprintf("in create frames storage \n");

	struct FrameInfo** frames = (struct FrameInfo**)kmalloc(numOfFrames * sizeof(struct FrameInfo *));

	if(frames == NULL || (void*) frames == (void*) -1)
		return NULL;

	return frames;

}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("create_share is not implemented yet");
	//Your Code is Here...

	struct Share* sharedObj = (struct Share*)kmalloc(size);

	if(sharedObj == NULL || (void*)sharedObj == (void*) -1)
		return NULL;
	int32 id = (int32)sharedObj & 0x7FFFFFFF;
	sharedObj->ID = id;

	int numberOfFrames = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	sharedObj->framesStorage = (create_frames_storage(numberOfFrames));
	if(sharedObj->framesStorage == NULL || (void*)sharedObj->framesStorage == (void*) -1){
		kfree(sharedObj);
		return NULL;
	}
//	for(int i=0; i<numberOfFrames; i++){
//
//		uint32 va = (uint32)sharedObj;
//		uint32 *ptr_page_table;
//
//		if(get_page_table(ptr_page_directory,va,&ptr_page_table) == TABLE_NOT_EXIST)
//			return NULL;
//
//		sharedObj->framesStorage[i] = get_frame_info(ptr_page_directory, va, &ptr_page_table);
//
//		va += PAGE_SIZE;
//	}

	strcpy(sharedObj->name, shareName);
	sharedObj->size = size;
	sharedObj->ownerID = ownerID;
	sharedObj->references = 1;
	sharedObj->isWritable = isWritable;
	bool lock_is_held = holding_spinlock(&AllShares.shareslock);

	// should i add to the list myself or it's added internally
//	if (!lock_is_held)
//		acquire_spinlock(&AllShares.shareslock);
//	LIST_INSERT_TAIL(AllShares.shares_list, sharedObj);
//	if (!lock_is_held)
//		release_spinlock(&AllShares.shareslock);

	return sharedObj;

}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("get_share is not implemented yet");
	//Your Code is Here...

}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("createSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("getSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("free_share is not implemented yet");
	//Your Code is Here...

}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	//Your Code is Here...

}
