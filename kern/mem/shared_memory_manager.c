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



	struct FrameInfo** frames = (struct FrameInfo**)kmalloc(numOfFrames * sizeof(struct FrameInfo *));

	if(frames == NULL || (void*) frames == (void*) -1){
		return NULL;
	}
	for(int i=0;i<numOfFrames;i++){
		frames[i]=NULL;
	}


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
	// Dynamically allocate memory for the Share structure
	    struct Share* obj = (struct Share*)kmalloc(sizeof(struct Share));
	    if (obj == NULL || (void*)obj == (void*)-1)
	    {
	        panic("Failed to allocate memory for Share object");
	        return NULL;
	    }

	    obj->ownerID = ownerID;

	    // Copy shareName safely into the name field
	    strncpy(obj->name, shareName, sizeof(obj->name) - 1);
	    obj->name[sizeof(obj->name) - 1] = '\0'; // Ensure null-termination

	    obj->size = size;
	    obj->references = 1;
	    obj->isWritable = isWritable;
	    // Calculate the number of frames required
	    int numberOfFrames = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

	    // Create framesStorage and handle errors
	    obj->framesStorage = create_frames_storage(numberOfFrames);
	    if (obj->framesStorage == NULL)
	    {
	        panic("Failed to allocate memory for framesStorage");
	        kfree(obj);
	        return NULL;
	    }


	    return obj;


}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name) {
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
//	panic("get_share is not implemented yet");
	//Your Code is Here...
	struct Share *obj = NULL;
	//lock allShares list
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);

	if (!lock_already_held) {
		acquire_spinlock(&AllShares.shareslock);
	}

	LIST_FOREACH(obj, &AllShares.shares_list)
	{
		if (strcmp(name, obj->name) == 0 && ownerID == obj->ownerID) {
			if (!lock_already_held)
				release_spinlock(&AllShares.shareslock);
			return obj;
		}
	}
	if (!lock_already_held)
		release_spinlock(&AllShares.shareslock);
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
	if(num_of_pages > LIST_SIZE(&MemFrameLists.free_frame_list)){
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
		map_frame(myenv->env_page_directory, ptr_frame_info, start_va, PERM_WRITEABLE| PERM_USER);
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

//	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
//	if (!lock_already_held) {
//		acquire_spinlock(&AllShares.shareslock);
//	}

	if(my_shared_object==NULL){
//		if (!lock_already_held) {
//						release_spinlock(&AllShares.shareslock);
//			}
		return E_SHARED_MEM_NOT_EXISTS;

	}
	struct FrameInfo** frames_shared = my_shared_object->framesStorage;
	uint8 write = my_shared_object->isWritable;
	uint32 num_of_frames = ROUNDUP(my_shared_object->size, PAGE_SIZE) / PAGE_SIZE;
	//does this need lock
	for(int i=0;i<num_of_frames;i++){


		if(write)
			map_frame(myenv->env_page_directory, frames_shared[i], (uint32)virtual_address, PERM_WRITEABLE | PERM_USER);
		else
			map_frame(myenv->env_page_directory, frames_shared[i], (uint32)virtual_address,  PERM_USER);


		virtual_address+=PAGE_SIZE;



	}
	my_shared_object->references = my_shared_object->references + 1;
//	if (!lock_already_held) {
//		release_spinlock(&AllShares.shareslock);
//	}
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
	//panic("free_share is not implemented yet");
	//Your Code is Here...
	LIST_REMOVE(&AllShares.shares_list, ptrShare);
	kfree((void*)ptrShare->framesStorage);
	kfree((void*)ptrShare);
}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("freeSharedObject is not implemented yet");
	//Your Code is Here...
	//lock TODO ????

	struct Share* share_ptr;
	struct Share* found_share_ptr=NULL;
	struct Env* myenv = get_cpu_proc(); //The calling environment
	//searching for share object with its id
	int number_of_page_tables = 0;
	uint32 page_tables[1<<10];
	uint32 virt_addrs[1<<10];
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
	if (!lock_already_held) {
		acquire_spinlock(&AllShares.shareslock);
	}
	LIST_FOREACH(share_ptr, &AllShares.shares_list){
		//if found unmap it from current process and remove page tables if they are empty
		if(share_ptr->ID == sharedObjectID){
			found_share_ptr = share_ptr;
			break;
		}
	}

	if(found_share_ptr==NULL){
		if (!lock_already_held) {
						release_spinlock(&AllShares.shareslock);
			}
		return E_SHARED_MEM_NOT_EXISTS;

	}
	found_share_ptr->references--;

	//if this was the last reference then delete share obj
	if(found_share_ptr->references==0){
		free_share(found_share_ptr);
	}
	int size = found_share_ptr->size;
	int num_of_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	uint32 va = (uint32) startVA;
	for(int i=0;i<num_of_pages;i++){
		//making a set of page tables to check if they are emtpy later
		uint32 *page_table;
		get_page_table(myenv->env_page_directory, va, &page_table);
		bool f=0;
		for(int j=0;j<number_of_page_tables;j++){
			if(page_tables[j] == (uint32)page_table){
				f=1;
				break;
			}
		}
		if(!f){
			page_tables[number_of_page_tables] = (uint32) page_table;
			virt_addrs[number_of_page_tables] = va;
			number_of_page_tables++;
		}
		///////////////////////////////////////////////////////////////
		unmap_frame(myenv->env_page_directory, va);
		va+=PAGE_SIZE;
	}
	//check on page tables
	for(int i=0;i<number_of_page_tables;i++){
		uint32 *page_table_ptr = (uint32*)page_tables[i];
		bool empty=1;
		for(int j=0;j<(1<<10);j++){
			if(page_table_ptr[j]!=0){
				empty = 0;
				break;
			}
		}
		//if a page table is empty remove it
		if(empty){
			uint32 * page_dir = myenv->env_page_directory;
			kfree((void*)page_tables[i]);
			pd_clear_page_dir_entry(page_dir, virt_addrs[i]);
		}
	}
	if (!lock_already_held) {
					release_spinlock(&AllShares.shareslock);
		}
	tlbflush();

	return 0;
}
