#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/environment_definitions.h>

#include <kern/semaphore_manager.h>
#include <kern/shared_memory_manager.h>
#include <kern/memory_manager.h>
#include <kern/syscall.h>
#include <kern/kheap.h>
//2017

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] Create "shares" array:
//===========================
//Dynamically allocate the array of shared objects
//initialize the array of shared objects by 0's and empty = 1
void create_shares_array(uint32 numOfElements)
{

	shares = kmalloc(numOfElements*sizeof(struct Share));
	if (shares == NULL)
	{
		panic("Kernel runs out of memory\nCan't create the array of shared objects.");
	}
	for (int i = 0; i < MAX_SHARES; ++i)
	{
		memset(&(shares[i]), 0, sizeof(struct Share));
		shares[i].empty = 1;
	}
}

//===========================
// [2] Allocate Share Object:
//===========================
//Allocates a new (empty) shared object from the "shares" array
//It dynamically creates the "framesStorage"
//Return:
//	a) if succeed:
//		1. allocatedObject (pointer to struct Share) passed by reference
//		2. sharedObjectID (its index in the array) as a return parameter
//	b) E_NO_SHARE if the the array of shares is full (i.e. reaches "MAX_SHARES")
int allocate_share_object(struct Share **allocatedObject)
{
	int32 sharedObjectID = -1 ;
	for (int i = 0; i < MAX_SHARES; ++i)
	{
		if (shares[i].empty)
		{
			sharedObjectID = i;
			break;
		}
	}

	if (sharedObjectID == -1)
	{
		//try to increase double the size of the "shares" array
		if (USE_KHEAP == 1)
		{
			shares = krealloc(shares, 2*MAX_SHARES);
			if (shares == NULL)
			{
				*allocatedObject = NULL;
				return E_NO_SHARE;
			}
			else
			{
				sharedObjectID = MAX_SHARES;
				MAX_SHARES *= 2;
			}
		}
		else
		{
			*allocatedObject = NULL;
			return E_NO_SHARE;
		}
	}

	*allocatedObject = &(shares[sharedObjectID]);
	shares[sharedObjectID].empty = 0;

	if (USE_KHEAP == 1)
	{
		shares[sharedObjectID].framesStorage = kmalloc(PAGE_SIZE);
		if (shares[sharedObjectID].framesStorage == NULL)
		{
			panic("Kernel runs out of memory\nCan't create the framesStorage.");
		}
		memset(shares[sharedObjectID].framesStorage, 0, PAGE_SIZE);
	}
	return sharedObjectID;
}

//=========================
// [3] Get Share Object ID:
//=========================
//Search for the given shared object in the "shares" array
//Return:
//	a) if found: SharedObjectID (index of the shared object in the array)
//	b) else: E_SHARED_MEM_NOT_EXISTS
int get_share_object_ID(int32 ownerID, char* name)
{
	int i=0;
	for(; i< MAX_SHARES; ++i)
	{
		if (shares[i].empty)
			continue;

		if(shares[i].ownerID == ownerID && strcmp(name, shares[i].name)==0)
		{
			return i;
		}
	}
	return E_SHARED_MEM_NOT_EXISTS;
}

//=========================
// [4] Delete Share Object:
//=========================
//delete the given sharedObjectID from the "shares" array
//Return:
//	a) 0 if succeed
//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists
int free_share_object(uint32 sharedObjectID)
{
	if (sharedObjectID >= MAX_SHARES)
		return E_SHARED_MEM_NOT_EXISTS;

	//panic("deleteSharedObject: not implemented yet");
	clear_frames_storage(shares[sharedObjectID].framesStorage);
	if (USE_KHEAP == 1)
		kfree(shares[sharedObjectID].framesStorage);

	memset(&(shares[sharedObjectID]), 0, sizeof(struct Share));
	shares[sharedObjectID].empty = 1;

	return 0;
}
//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=========================
// [1] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//cprintf("\n\n");
	//TODO: [PROJECT 2017 - [6] Shared Variables: Creation] createSharedObject() [Kernel Side]
	// your code is here, remove the panic and write your code
	//panic("createSharedObject() is not implemented yet...!!");
	struct Env* myenv = curenv; //The calling environment

	if(get_share_object_ID(ownerID,shareName) != E_SHARED_MEM_NOT_EXISTS )
		return E_SHARED_MEM_EXISTS ;

	struct Share *share_item ;
	int share_item_idx = allocate_share_object(&share_item) ,  NEED_FRAMES = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE ;
	if( share_item_idx == E_NO_SHARE)
		return E_NO_SHARE ;

	struct Frame_Info * ptr_frame = NULL ;
  //  cprintf("Created++> ShareObjName:%s ,shaeObjID:%d , NeededFrames:- %d \n" , shareName,share_item_idx ,NEED_FRAMES);
  for(int i = 0  ; i < NEED_FRAMES ; i++,virtual_address += PAGE_SIZE)
   {
	  allocate_frame(&ptr_frame) ;
	  map_frame(myenv->env_page_directory ,ptr_frame ,(void*)virtual_address,PERM_USER|PERM_WRITEABLE);
	  add_frame_to_storage(shares[share_item_idx].framesStorage , ptr_frame,i);
	//  	  cprintf("FrameNm :%d ,ref: %d , indexinstorage:- %d \n",to_frame_number(ptr_frame) ,ptr_frame->references, i);

   }
 // cprintf("=========== End Of Create =======\n\n");
	share_item->isWritable = isWritable ; strcpy(share_item->name , shareName);
	share_item->size = size ; share_item->ownerID = ownerID ;
	share_item->references = 1 ;
	return share_item_idx;
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//TODO: [PROJECT 2017 - [6] Shared Variables: GetSize] getSizeOfSharedObject()
	// your code is here, remove the panic and write your code
	//panic("getSizeOfSharedObject() is not implemented yet...!!");

	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	 int idx =  get_share_object_ID(ownerID , shareName) ;
       if(idx == E_SHARED_MEM_NOT_EXISTS)
    	   return E_SHARED_MEM_NOT_EXISTS ;

       return shares[idx].size ;
	//change this "return" according to your answer
}

//======================
// [3] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	 //  cprintf("ENtered <=================== \n");

	//TODO: [PROJECT 2017 - [6] Shared Variables: Get] getSharedObject() [Kernel Side]
	// your code is here, remove the panic and write your code
	struct Env* myenv = curenv; //The calling environment

	 int shared_item_idx = get_share_object_ID(ownerID ,shareName) ;
     if(shared_item_idx == E_SHARED_MEM_NOT_EXISTS)
    	 return E_SHARED_MEM_NOT_EXISTS ;

     struct Frame_Info *ptr_frame = NULL ;
     int NEEDED_FRAMES = ROUNDUP(shares[shared_item_idx].size , PAGE_SIZE)/PAGE_SIZE ;

  for(int i = 0 ; i<NEEDED_FRAMES; i++,virtual_address += PAGE_SIZE )
    {
     ptr_frame =  get_frame_from_storage(shares[shared_item_idx].framesStorage ,i);

     if(shares[shared_item_idx].isWritable == 1)
     map_frame(myenv->env_page_directory,ptr_frame,virtual_address,PERM_WRITEABLE|PERM_USER);
     else
     map_frame(myenv->env_page_directory,ptr_frame,virtual_address,PERM_USER);
   }
    shares[shared_item_idx].references++;
     //change this "return" according to your answer
  return shared_item_idx;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===================
// Free Share Object:
//===================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	struct Env* myenv = curenv; //The calling environment

	//TODO: [PROJECT 2017 - BONUS4] Free Shared Variable [Kernel Side]
	// your code is here, remove the panic and write your code
	panic("freeSharedObject() is not implemented yet...!!");

	// This function should free (delete) the shared object from the User Heapof the current environment
	// If this is the last shared env, then the "frames_store" should be cleared and the shared object should be deleted
	// RETURN:
	//	a) 0 if success
	//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists


	// Steps:
	//	1) Get the shared object from the "shares" array (use get_share_object_ID())
	//	2) Unmap it from the current environment "myenv"
	//	3) If one or more table becomes empty, remove it
	//	4) Update references
	//	5) If this is the last share, delete the share object (use free_share_object())
	//	6) Flush the cache "tlbflush()"

	//change this "return" according to your answer
	return 0;
}
