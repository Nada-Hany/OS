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
	panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

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
	panic("create_share is not implemented yet");
	//Your Code is Here...

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
//	panic("get_share is not implemented yet");
	//Your Code is Here...

	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held)
		acquire_spinlock(&AllShares.shareslock);

	if(LIST_SIZE(&AllShares.shares_list) == 0)
		return NULL;

	if (!lock_already_held)
		release_spinlock(&AllShares.shareslock);

	char objName[64];
	strcpy(objName, name);

	struct Share* obj = NULL;
	LIST_FOREACH(obj, &AllShares.shares_list){
		if(obj->ownerID == ownerID && obj->name == objName)
			return obj;
	}
	return NULL;

}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size,
		uint8 isWritable, void* virtual_address) {
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...
	struct Env* myenv = get_cpu_proc(); //The calling environment

	//search if obj is already exist
	if (get_share(ownerID, shareName) != NULL) {
		return E_SHARED_MEM_EXISTS;
	}

	uint32 num_of_pages = ROUNDUP(size,PAGE_SIZE) / PAGE_SIZE;

	// TODO check if no mem
	if(num_of_pages<LIST_SIZE(&MemFrameLists.free_frame_list)){
		return E_NO_SHARE;
	}

	//create and init shared obj
	struct Share* shared_obj = create_share(ownerID, shareName, size,
			isWritable);

	//add shared obj to shared list (lock -> insert ->release)
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held) {
		acquire_spinlock(&AllShares.shareslock);
	}

	LIST_INSERT_HEAD(&AllShares.shares_list, shared_obj);

	if (!lock_already_held) {
		release_spinlock(&AllShares.shareslock);
	}

	uint32 start_va = (uint32) virtual_address;
	int index_of_framesStorage = 0;
	while (num_of_pages) {

		struct FrameInfo *ptr_frame_info;

		// allocate new frame ,map this frame to va in env_page_dir , add this frame to frames storage
		allocate_frame(&ptr_frame_info);
		map_frame(myenv->env_page_directory, ptr_frame_info, start_va,
		PERM_WRITEABLE);

		shared_obj->framesStorage[index_of_framesStorage] = ptr_frame_info;

		start_va += PAGE_SIZE;
		num_of_pages--;
		index_of_framesStorage++;
	}
	shared_obj->ID=(uint32)virtual_address& 0x7FFFFFFF;

	return shared_obj->ID;
}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share* my_shared_object = get_share(ownerID, shareName);
	if(my_shared_object==NULL){
		return E_SHARED_MEM_NOT_EXISTS;
	}
	struct FrameInfo** frames_shared = my_shared_object->framesStorage;
	uint8 write = my_shared_object->isWritable;
	uint32 num_of_frames = ROUNDUP(my_shared_object->size, PAGE_SIZE) / PAGE_SIZE;
	for(int i=0;i<num_of_frames;i++){
		map_frame(myenv->env_page_directory, frames_shared[i], (uint32)virtual_address, write);
		virtual_address+=PAGE_SIZE;
	}
	my_shared_object->references = my_shared_object->references + 1;

	return my_shared_object->ID;
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
