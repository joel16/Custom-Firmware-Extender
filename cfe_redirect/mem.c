/*
 *	untitled.c is part of HostCore
 *	Copyright (C) 2008  Poison
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *	Description:	
 *	Author:			Poison <hbpoison@gmail.com>
 *	Date Created:	2008-04-16
 */

#include <pspkernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "log.h"

static int block_id[MAX_BLOCK], count = 0;
static void * block_ptr[MAX_BLOCK];

void * memAlloc( int size, int partition )
{
	if ( count >= MAX_BLOCK )
		return NULL;
	char name[16];
	sprintf( name, "cusAlloc_%d", count );
	block_id[count] = sceKernelAllocPartitionMemory( partition, name, PSP_SMEM_Low, size, NULL );
	if ( block_id[count] < 0 )
	{
		log( "Error alloc memory size %08x on partion %d\n", size, partition );
		return NULL;
	}
	block_ptr[count] = sceKernelGetBlockHeadAddr( block_id[count] );
	return block_ptr[count ++];
}

int memFree( void * ptr )
{
	int i;
	for ( i = 0; i < count; i ++ )
	{
		if ( block_ptr[i] == ptr )
			return sceKernelFreePartitionMemory( block_id[i] );
	}
	return -1;
}
