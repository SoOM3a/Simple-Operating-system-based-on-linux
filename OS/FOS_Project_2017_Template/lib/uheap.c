#include <inc/lib.h>
// malloc()
//	This function use FIRST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

struct USER_HEAP_ADDRESSES
{
	uint32 begin ,sz ;
};

uint32 First_addrInHeap = USER_HEAP_START , Nshares = 0; // indx=0  ;
struct USER_HEAP_ADDRESSES address[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];
bool UHeap_Pages[(USER_HEAP_MAX - USER_HEAP_START)/PAGE_SIZE] = {0};
bool UHeap_sharedObj_Pages[(USER_HEAP_MAX - USER_HEAP_START)/PAGE_SIZE] = {0};

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

uint32  Get_FreeaddUheap(uint32 size ,  bool UHeap_Pages[])
{
	uint32 i, start;
	uint32 sz = 0;
	uint32 Uheap_addresses = USER_HEAP_START , inc =PAGE_SIZE;
	bool flag = 0,found=0;
	for(i = USER_HEAP_START; i < USER_HEAP_MAX; i+= inc)
	{
		inc=PAGE_SIZE ;
		if(sz >= size)
		{
			Uheap_addresses = start;
			found = 1;
			break;
		}
		if(UHeap_Pages[(i - USER_HEAP_START)/PAGE_SIZE] == 0 && flag)
			sz += PAGE_SIZE;
		else if(UHeap_Pages[(i - USER_HEAP_START)/PAGE_SIZE] == 0 && !flag)
		{

			flag = 1;
			start = i;
			sz += PAGE_SIZE;
		}
		else
		{
			if(i%PAGE_SIZE==0 && address[((i - USER_HEAP_START)/PAGE_SIZE)].sz>0)
				inc =address[((i - USER_HEAP_START)/PAGE_SIZE)].sz ;
			flag = 0;
			sz = 0;
		}
	}
	if(found == 0 && sz >= size)
		Uheap_addresses = start;
	else if(found == 0)
		return -1;
return  Uheap_addresses;
}
void  mark_taken(uint32 Uheap_addresses , uint32 size,bool UHeap_Pages[])
{
	for(uint32 j = 0; j < size/PAGE_SIZE; j++)
			UHeap_Pages[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)+j] = 1;

}

void* malloc(uint32 size)
{
	//TODO: [PROJECT 2017 - [5] User Heap] malloc() [User Side]
	size = ROUNDUP(size, PAGE_SIZE);
    uint32  Uheap_addresses = Get_FreeaddUheap(size , UHeap_Pages) ;
     if(Uheap_addresses == -1 )return NULL ;
  //   address[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)].begin = Uheap_addresses;
    // 	address[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)].sz = size;
   uint32 ret_add = Uheap_addresses;
	sys_allocateMem(Uheap_addresses, size);

	mark_taken(Uheap_addresses , size,UHeap_Pages);

	address[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)].begin = Uheap_addresses;
	address[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)].sz = size;
	return (void*)address[((ret_add - USER_HEAP_START)/PAGE_SIZE)].begin;
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT 2017 - [6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL
   	     size = ROUNDUP(size, PAGE_SIZE);
   	     if(Nshares >= sys_getMaxShares() )
   	    	 return NULL ;
   	    uint32  Uheap_addresses = Get_FreeaddUheap(size,UHeap_Pages) ;

   	     if(Uheap_addresses < 0 )
	    	 return NULL ;

	int idx = sys_createSharedObject(sharedVarName,size,isWritable,(void*)Uheap_addresses);
    if(idx < 0)
    	return NULL ;

	mark_taken(Uheap_addresses ,size,UHeap_Pages);
	Nshares++;

	address[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)].begin = Uheap_addresses;
	address[((Uheap_addresses - USER_HEAP_START)/PAGE_SIZE)].sz = size;
	return (void*)Uheap_addresses;
}


void* sget(int32 ownerEnvID, char *sharedVarName)
{

	//TODO: [PROJECT 2017 - [6] Shared Variables: Get] sget() [User Side]
	//sys_waitSemaphore(ownerEnvID , sharedVarName);
	int shared_item_sz =  sys_getSizeOfSharedObject(ownerEnvID , sharedVarName) ;
	if(shared_item_sz == E_SHARED_MEM_NOT_EXISTS)
		return NULL ;
	shared_item_sz  = ROUNDUP(shared_item_sz , PAGE_SIZE);
	uint32 Uheap_addresses  =  Get_FreeaddUheap(shared_item_sz ,UHeap_Pages);  // UHeap_sharedObj_Pages);

	if(Uheap_addresses == -1 )return NULL ;

	uint32 ret  = Uheap_addresses ;
   int shared_item_idx =  sys_getSharedObject(ownerEnvID,sharedVarName,(void*)Uheap_addresses);

   if(shared_item_idx == E_SHARED_MEM_NOT_EXISTS)
	   return NULL;
   mark_taken(Uheap_addresses,shared_item_sz,UHeap_Pages);

   address[((ret - USER_HEAP_START)/PAGE_SIZE)].begin = Uheap_addresses;
   	address[((ret - USER_HEAP_START)/PAGE_SIZE)].sz = shared_item_sz;
   	return  (void* ) ret ;

}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2017 - [5] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	//you should get the size of the given allocation using its address
	//you need to call sys_freeMem()

	for(int i = 0 ; i < address[(((uint32)virtual_address - USER_HEAP_START)/PAGE_SIZE)].sz/PAGE_SIZE ;i++)
	{
		UHeap_Pages[(((uint32)virtual_address - USER_HEAP_START)/PAGE_SIZE)+i] = 0;
	}
	sys_freeMem((uint32)virtual_address , address[(((uint32)virtual_address - USER_HEAP_START)/PAGE_SIZE)].sz);
	address[(((uint32)virtual_address - USER_HEAP_START)/PAGE_SIZE)].begin = 0;
	address[(((uint32)virtual_address - USER_HEAP_START)/PAGE_SIZE)].sz = 0;
	//refer to the project presentation and documentation for details
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT 2017 - BONUS4] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2017 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

}
