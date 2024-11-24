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
//	memset(frames, 0, numOfFrames * sizeof(struct FrameInfo));
	if(frames == NULL || (void*) frames == (void*) -1){
		cprintf("return null in create frame storage\n");
		return NULL;
	}
for(int i=0;i<numOfFrames;i++){
	frames[i]=NULL;
}
	cprintf("end of create frame storage func \n");

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

	/*struct Share* obj;
	strcpy(obj->name, shareName);
	obj->size = size;
	obj->ownerID = ownerID;
	obj->references = 1;
	obj->isWritable = isWritable;
	cprintf("end of helper create share func \n");
	return obj;*/



	/*cprintf("in create share helper func\n");
	struct Share* sharedObj = (struct Share*)kmalloc(size);
	cprintf("frames after allocating share object  = %d \n", LIST_SIZE( &MemFrameLists.free_frame_list));
	cprintf("%d \n\n", LIST_SIZE(&MemFrameLists.free_frame_list));
	if(sharedObj == NULL || (void*)sharedObj == (void*) -1)
		return NULL;

	int32 id = (int32)sharedObj & 0x7FFFFFFF;
	sharedObj->ID = id;

	int numberOfFrames = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	sharedObj->framesStorage = (create_frames_storage(numberOfFrames));
	cprintf("frames after allocatinf frames storage = %d", LIST_SIZE( &MemFrameLists.free_frame_list));
	if(sharedObj->framesStorage == NULL || (void*)sharedObj->framesStorage == (void*) -1){
		cprintf("in if condition\n");
		kfree(sharedObj);
		return NULL;
	}

	strcpy(sharedObj->name, shareName);
	sharedObj->size = size;
	sharedObj->ownerID = ownerID;
	sharedObj->references = 1;
	sharedObj->isWritable = isWritable;
	cprintf("end of helper create share func \n");
	return sharedObj;*/
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

	    cprintf("Share object created successfully: ID=%d, Name=%s, Size=%d, Writable=%d\n",
	            obj->ID, obj->name, obj->size, obj->isWritable);

	    return obj;

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
	/*cprintf("in get share func \n");

	 char objName[64];
	 strcpy(objName, name);

	 struct Share* obj = NULL;
	 LIST_FOREACH(obj, &AllShares.shares_list){
	 int size = strlen(name);
	 bool sameName = 1;
	 for(int i=0; i<size;i++){
	 if(objName[i] != obj->name[i])
	 sameName = 0;
	 }
	 if(obj->ownerID == ownerID && sameName)
	 return obj;
	 }*/
	struct Share *obj = NULL;
	//lock allShares list
	acquire_spinlock(&AllShares.shareslock);

	LIST_FOREACH(obj, &AllShares.shares_list)
	{
		if (strcmp(name, obj->name) == 0 && ownerID == obj->ownerID) {
			release_spinlock(&AllShares.shareslock);
			return obj;
		}
	}
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
//	cprintf("in create share obj in user side\n");
	//search if obj is already exist
	if (get_share(ownerID, shareName) != NULL) {
		cprintf("shared already exists\n");
		return E_SHARED_MEM_EXISTS;
	}
//	cprintf("after get share call\n");
	uint32 num_of_pages = ROUNDUP(size,PAGE_SIZE) / PAGE_SIZE;

	// TODO check if no mem
	if(num_of_pages > LIST_SIZE(&MemFrameLists.free_frame_list)){
		cprintf("no mem for shared obj \n");
		return E_NO_SHARE;
	}

	//create and init shared obj
	struct Share* shared_obj = create_share(ownerID, shareName, size,
			isWritable);

	//add shared obj to shared list (lock -> insert ->release)
	bool lock_already_held = holding_spinlock(&AllShares.shareslock);
//	cprintf("holding lock val = %d \n", lock_already_held);

	if (!lock_already_held) {
//		cprintf("before acquiring lock\n");
		acquire_spinlock(&AllShares.shareslock);
	}

	LIST_INSERT_HEAD(&AllShares.shares_list, shared_obj);

	if (!lock_already_held) {
		release_spinlock(&AllShares.shareslock);
	}
	cprintf("----------->va in create shared object = %x \n", virtual_address);
	uint32 start_va = (uint32) virtual_address;
	int index_of_framesStorage = 0;
	while (num_of_pages) {

		struct FrameInfo *ptr_frame_info;

		// allocate new frame ,map this frame to va in env_page_dir , add this frame to frames storage
		allocate_frame(&ptr_frame_info);
		map_frame(myenv->env_page_directory, ptr_frame_info, start_va,PERM_WRITEABLE | PERM_USER);
		//cprintf("--->frame %x  ,  va %x\n",ptr_frame_info, start_va);
		shared_obj->framesStorage[index_of_framesStorage] = ptr_frame_info;

		start_va += PAGE_SIZE;
		num_of_pages--;
		index_of_framesStorage++;
	}
	shared_obj->ID=(uint32)virtual_address& 0x7FFFFFFF;
	cprintf("availible frames after creating the shared obj = %d \n", LIST_SIZE( &MemFrameLists.free_frame_list));
	cprintf("end of create shared obj\n");
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

	cprintf("------>in get shared obj kernel side \n");

	struct Env* myenv = get_cpu_proc(); //The calling environment
	struct Share* my_shared_object = get_share(ownerID, shareName);
	if(my_shared_object==NULL){
		return E_SHARED_MEM_NOT_EXISTS;
	}
	struct FrameInfo** frames_shared = my_shared_object->framesStorage;
	uint8 write = my_shared_object->isWritable;
	uint32 num_of_frames = ROUNDUP(my_shared_object->size, PAGE_SIZE) / PAGE_SIZE;

	for(int i=0;i<num_of_frames;i++){

//		struct FrameInfo *ptr_frame_info = frames_shared[i];
//		allocate_frame(&ptr_frame_info);

		//map_frame(myenv->env_page_directory, frames_shared[i], (uint32)virtual_address, PERM_WRITEABLE | PERM_USER);
		if(write)
			map_frame(myenv->env_page_directory, frames_shared[i], (uint32)virtual_address, PERM_WRITEABLE | PERM_USER);
		else
			map_frame(myenv->env_page_directory, frames_shared[i], (uint32)virtual_address,  PERM_USER);

		//cprintf("--->frame %x  ,  va %x  ,  value %d\n",frames_shared[i],virtual_address,*frames_shared[i]);
		virtual_address+=PAGE_SIZE;

		frames_shared[i]->references++;

	}
	my_shared_object->references = my_shared_object->references + 1;
	cprintf("end of get shared obj kernel side \n");

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
	//panic("freeSharedObject is not implemented yet");
	//Your Code is Here...
	struct Share* share_ptr;
	struct Env* myenv = get_cpu_proc(); //The calling environment
	LIST_FOREACH(share_ptr, &AllShares.shares_list){
		if(share_ptr->ID == sharedObjectID){

		}
	}
}
