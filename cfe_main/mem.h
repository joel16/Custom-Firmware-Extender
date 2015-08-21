#ifndef __mem_h
#define __mem_h

SceUID memid;

void *Kmalloc(int partition, SceSize  size, SceUID *memid);
void Kfree( SceUID memid);
void Kmemset( void *p, u8 c, int len);

#endif

