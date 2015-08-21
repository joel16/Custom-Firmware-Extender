/*
 *	log.c is part of host2ms
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
 *	Date Created:	2008-04-05
 */

#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils.h"

extern int init_key;

#if _DEBUG >= 1
int log_running = 0;
char lstr[128];
int printlog( const char * str )
{
	log_running = 1;
	if(init_key == PSP_INIT_KEYCONFIG_GAME)
	{
		int fd = sceIoOpen( "ms0:/seplugins/cfe/log.txt", PSP_O_RDWR | PSP_O_CREAT | PSP_O_APPEND, 0777 );
		sceIoWrite( fd, str, strlen( str ) );
		sceIoClose( fd );
	}
	log_running = 0;
	return 0;
}

int printFileArg( PspIoDrvFileArg * arg )
{
	log( "unk1: %08x\nfs_num: %08x\ndrv: %08x\nunk2: %08x\narg: %08x\n",
		( unsigned int )arg->unk1, ( unsigned int )arg->fs_num, ( unsigned int )arg->drv, ( unsigned int )arg->unk2, ( unsigned int )arg->arg );
	if ( !arg->arg )
		return 0;
	unsigned int * tmp = arg->arg;
	log( "arg0: %08x\narg1: %08x\narg2: %08x\narg3: %08x\n",
	tmp[0], tmp[1], tmp[2], tmp[3] );
	return 0;
}

void dumpRange( const char * name, void * start, int len )
{
	sceIoMkdir( "ms0:/Dump", 0777 );
	sprintf( lstr, "ms0:/Dump/%s.bin", name );
	int fd = sceIoOpen( lstr, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_RDWR, 0777 );
	sceIoWrite( fd, start, len );
	sceIoClose( fd );
}

void dumpKmem()
{
	dumpRange( "kmem", ( void * )0x88000000, 0x00400000 );
}

void dumpUmem()
{
	dumpRange( "umem", ( void * )0x08800000, 0x01800000 );
}

void dumpThreadList()
{
	int t_ids[100], t_count, i;
	memset( t_ids, 0, 400 );
	sceKernelGetThreadmanIdList( SCE_KERNEL_TMID_Thread, t_ids, 400, &t_count );
	log( "%d thread found!\n", t_count );
	for( i = 0; i < t_count; i ++ )
	{
		SceKernelThreadInfo info;
		memset( &info, 0, sizeof( SceKernelThreadInfo ) );
		info.size = sizeof( SceKernelThreadInfo );
		if ( sceKernelReferThreadStatus( t_ids[i], &info ) == 0 )
		{
			log( "thread id %08x name %s\n", t_ids[i], info.name );
		}
	}
}

void dumpModuleList()
{
	int m_ids[100], m_count, i;
	memset( m_ids, 0, 400 );
	sceKernelGetModuleIdList( m_ids, 400, &m_count );
	log( "%d module found!\n", m_count );
	for( i = 0; i < m_count; i ++ )
	{
		tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByUID( m_ids[i] );
		if ( pMod )
			log( "module id %08x name %s\n", m_ids[i], pMod->modname );
	}
}

void dumpMemPartitionInfo()
{
	int i;
	for( i = 1; i < 6; i ++ )
	{
		PspSysmemPartitionInfo info;
		memset( &info, 0, sizeof( PspSysmemPartitionInfo ) );
		info.size = sizeof( PspSysmemPartitionInfo );
		sceKernelQueryMemoryPartitionInfo( i, &info );
		log( "partition %d start %08x size %08x free %08x attr %x\n", i, info.startaddr, info.memsize, sceKernelPartitionTotalFreeMemSize( i ), info.attr );
	}
}
#endif
