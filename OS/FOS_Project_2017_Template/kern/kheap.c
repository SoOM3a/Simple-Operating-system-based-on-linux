#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
struct KERNEL_HEAP_ADDRESSES
{
	uint32 begin ,sz ;
	int IS_EMPTY ;
};
uint32 firstFreeVAInKHeap = KERNEL_HEAP_START   ,   idx_ARRAY_KH_ADDRS = 0 ;
struct KERNEL_HEAP_ADDRESSES ARRAY_KH_ADDRS[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];


//=================================================================================//
//============================ REQUIRED FUNCTION ==================================//
//=================================================================================//
void *kamlloccall(unsigned int size)
{
		int EXISITED_PAGES = (KERNEL_HEAP_MAX - firstFreeVAInKHeap)/PAGE_SIZE ;
		if( EXISITED_PAGES <= 0 || EXISITED_PAGES < size)
		 return NULL ;

		ARRAY_KH_ADDRS[idx_ARRAY_KH_ADDRS].sz = size ;
		ARRAY_KH_ADDRS[idx_ARRAY_KH_ADDRS].begin = firstFreeVAInKHeap;
		ARRAY_KH_ADDRS[idx_ARRAY_KH_ADDRS].IS_EMPTY = 0;
		int PERM  =  0 ;
		PERM&= (~PERM_USER);
		PERM |= (PERM_WRITEABLE) ;
		struct Frame_Info* Frame ;
		for(int i = 0 ; i<size ;i++)
		{
	    allocate_frame(&Frame);
	    map_frame(ptr_page_directory,Frame,(void *)firstFreeVAInKHeap,PERM);
	    firstFreeVAInKHeap += PAGE_SIZE ;
		}
	return (void*)ARRAY_KH_ADDRS[idx_ARRAY_KH_ADDRS++].begin;
}

void *kamllocFF(unsigned int size)
{
	int flag = 0 ;
	uint32 add = KERNEL_HEAP_START  , va;
	int cnt =0 ;
	for(int i= 0 ; i < (KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE; i++)
	{
		uint32 * ptr = NULL ;
		get_page_table(ptr_page_directory , (void *)add , &ptr);
		struct Frame_Info *frame = get_frame_info(ptr_page_directory ,(void *)add ,&ptr );
		if(frame == NULL)
			cnt++;

		else
			cnt=0;

	if(cnt == size)
		{
		 va = add -((cnt-1) * PAGE_SIZE);
			flag = 1;
			break;
		}
		add+=PAGE_SIZE;
	}
	if(!flag) return NULL ;
firstFreeVAInKHeap =  va;
return kamlloccall(size) ;;
}

void * kmallocNf(unsigned int size)
{
	//cprintf("Next fit entered Fun  ptrAdd: %x  <====== \n" ,firstFreeVAInKHeap);
   uint32 Va[10000] = {0}   ,sz[1000]={0} ,  add = KERNEL_HEAP_START  , va , * ptr = NULL, idx = 0; ;
   int flag = 0 ,  cnt =0 ;
   	for(int i= 0 ; i < (KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE; i++)
   	{
   		struct Frame_Info *frame = get_frame_info(ptr_page_directory ,(void *)add ,&ptr );
   		if(frame == NULL)
   			cnt++;

   		if(cnt >= size)
   		   {
   		   	 Va[idx] = add -((cnt-1) * PAGE_SIZE);
   		   	 sz[idx] = cnt ;
   		   	 flag = 1;
   		   }
   	 if ( cnt != 0 && frame != NULL  )
   		{
   	    Va[idx] = add -(cnt * PAGE_SIZE);
   	    sz[idx] = cnt ;
   		idx++;
        cnt = 0 ;
   	 }
   		add+=PAGE_SIZE;
   	}

   	if(!flag)
       return NULL ;

   /*	cprintf("size of Free Mem Frames : %d  , Needd fremes: %d <============= \n" , idx ,size) ;
   	for(int i = 0 ; i<idx ; i++)
   		cprintf("Va : %x , sz: %d \n " , Va[i] , sz[i]);
   	*/
   	flag = 0  ; int indx = -1 ;
   	int  i = 0 ;
   	for( i = 0 ; i<idx ; i++)
   	{
   		if(sz[i]>= size && firstFreeVAInKHeap <= Va[i])
   		{
   			//cprintf("Va[i] : %x  and V[i]>firstFreeVAInKHeap   How !!?? \n" , Va[i]);
   			flag = 0 ;
   			break ;
   		}
   		else if (!flag && sz[i]>= size)
   		{
   			indx = i ;
   			flag = 1 ;
   		}
   	}
   if(!flag)
    firstFreeVAInKHeap = Va[i] ;
   else
   	firstFreeVAInKHeap = Va[indx] ;
  // cprintf("Choosen addr : %x \n" ,firstFreeVAInKHeap );
 //  cprintf("****************===============*************\n");
	return (void *)kamlloccall(size);
}
void * kmallocBF(unsigned int size)
{
	//cprintf("Next fit entered Fun  ptrAdd: %x  <====== \n" ,firstFreeVAInKHeap);
   uint32 Va[10000] = {0}   ,sz[1000]={0} ,  add = KERNEL_HEAP_START  , va , * ptr = NULL, idx = 0; ;
   int flag = 0 ,  cnt =0 ;
   	for(int i= 0 ; i < (KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE; i++)
   	{
   		struct Frame_Info *frame = get_frame_info(ptr_page_directory ,(void *)add ,&ptr );
   		if(frame == NULL)
   			cnt++;

   		if(cnt >= size)
   		   {
   		   	 Va[idx] = add -((cnt-1) * PAGE_SIZE);
   		   	 sz[idx] = cnt ;
   		   	 flag = 1;
   		   }
   	 if ( cnt != 0 && frame != NULL  )
   		{
   			Va[idx] = add -(cnt * PAGE_SIZE);
   			 sz[idx] = cnt ;
   		}
   	 if(frame != NULL && cnt !=0)
   	 {
   		idx++;
        cnt = 0 ;
   	 }
   		add+=PAGE_SIZE;
   	}

   	if(!flag)
       return NULL ;

   /*	cprintf("size of Free Mem Frames : %d  , Needd fremes: %d <============= \n" , idx ,size) ;
   	for(int i = 0 ; i<idx ; i++)
   		cprintf("Va : %x , sz: %d \n " , Va[i] , sz[i]);
   	*/
   	flag = 0  ; int indx = 0 ;
   	int  i = 0 ;
   	for( i = 0 ; i<idx ; i++)
   	{
   		if(sz[i] == size)
   		{
   			//cprintf("Va[i] : %x  and V[i]>firstFreeVAInKHeap   How !!?? \n" , Va[i]);
   			flag = 0 ;
   			break ;
   		}
   		else if (!flag && sz[i]>= size)
   		{
   			if(sz[indx] < sz[i])
   			{
   			indx = i ;
   			}
   			flag = 1 ;
   		}
   	}
   if(!flag)
    firstFreeVAInKHeap = Va[i] ;
   else
   	firstFreeVAInKHeap = Va[indx] ;
  // cprintf("Choosen addr : %x \n" ,firstFreeVAInKHeap );
 //  cprintf("****************===============*************\n");
	return (void *)kamlloccall(size);
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//NOTE: Allocation is continuous increasing virtual address
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	size = ROUNDUP(size , PAGE_SIZE) / PAGE_SIZE;
	if(isKHeapPlacementStrategyFIRSTFIT())
		return kamllocFF(size);
	else if(isKHeapPlacementStrategyNEXTFIT())
		return kmallocNf(size);
	else if(isKHeapPlacementStrategyBESTFIT())
	   return kmallocBF(size);

		return kamlloccall(size) ;

	//TODO: [PROJECT 2017 - BONUS1] Implement a Kernel allocation strategy
	// Instead of the continuous allocation/deallocation, implement both
	// FIRST FIT and NEXT FIT strategies
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	//change this "return" according to your answer
}

void kfree(void* virtual_address)
{
	int id = -1 ; uint32 va ;
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	uint32* ptr_page_table = NULL ;
	//virtual_address = ROUNDDOWN(virtual_address,PAGE_SIZE);
	for(int i = 0 ; i<idx_ARRAY_KH_ADDRS ;i++)
		if( ARRAY_KH_ADDRS[i].begin == ((uint32)virtual_address)  )
		{
	//		cprintf("get idx in aray : %d \n" , i);
			id = i ;
			break ;
		}
	struct Frame_Info* Frame =NULL;
	va = ARRAY_KH_ADDRS[id].begin ;
	ARRAY_KH_ADDRS[idx_ARRAY_KH_ADDRS].IS_EMPTY = 1 ;
	for(int i = 0 ; i<ARRAY_KH_ADDRS[id].sz;i++)
	{
	Frame = get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table);
	if(Frame != NULL)
	{
		//cprintf(" Now i :- %d \n" , i) ;

		unmap_frame(ptr_page_directory , (void*)va);
		if(Frame->references >= 1)
			free_frame(Frame);
	}
	va +=  PAGE_SIZE ;
	}
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kheap_virtual_address()
	// tstkphysaddr Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");
   uint32 *ptr_page_table =NULL ; unsigned int pa = -1 ;
	for(unsigned int  va = KERNEL_HEAP_START ; va < firstFreeVAInKHeap ;va+=PAGE_SIZE)
     {
    	 get_page_table(ptr_page_directory,(void*)va,&ptr_page_table);
    	 if(ptr_page_table != NULL )
    	 {
    		pa =  ((ptr_page_table[PTX(va)]>>12)<<12) | ((va<<20)>>20) ;
    		 if(pa == physical_address)
    			 return va ;
    	 }
     }

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");
    uint32 *ptr_page_table =NULL ; unsigned int pa = -1 ;
	get_page_table(ptr_page_directory,(void*)virtual_address,&ptr_page_table);
	    	 if(ptr_page_table != NULL )
	    	 {
	    		pa =  ((ptr_page_table[PTX(virtual_address)]>>12)<<12) | ((virtual_address<<20)>>20) ;
	    			 return pa ;
	    	 }
	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	return 0;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2017 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
	//	panic("krealloc() is not implemented yet...!!");
	uint32 * New_place ;
	if(new_size == 0 )
		kfree(virtual_address);
	new_size = ROUNDUP(new_size,PAGE_SIZE) / PAGE_SIZE;
		int EXISITED_PAGES = (KERNEL_HEAP_MAX - firstFreeVAInKHeap)/PAGE_SIZE ;
		if( EXISITED_PAGES <= 0 || EXISITED_PAGES < new_size)
		 return NULL ;
	if ( virtual_address == NULL)
		New_place = kmalloc(new_size) ;
	else
	{
	kfree((void*)virtual_address) ;
	New_place = kmalloc(new_size) ;
	}
	return New_place ;
}
