#include <pspkernel.h>
#include <pspsdk.h>
 
void *Kmalloc(int partition, SceSize  size, SceUID *memid)
{
   *memid = sceKernelAllocPartitionMemory(partition, "mybuffer", 0, size, NULL);
   /*this will alloc the ram you need:
   *arg1: in the kernel partition (1 or 3(mirror)) if you want user use 2
   *arg2: buffer name not much interesting if you only need to alloch usefull to search for allocations in the uid lists
   *arg3: this can be 0 => alloc in the lowest adress 1 alloc in the highest adress 2 alloc in a user determined adress
   *arg4: num*byte of space to alloc
   *arg5: if arg3 = 2 adress to alloc to
   *
   *but you aren't done you need the adress were your allocation starts :)
   */
   return( sceKernelGetBlockHeadAddr(*memid) );

}

void Kfree( SceUID memid)
{
   //then to free it:
   sceKernelFreePartitionMemory(memid);
}

void Kmemset( void *p, u8 c, int len)
{
	int i;
	for( i=0; i<len; i++)
		((u8 *)p)[i] = c;
}
