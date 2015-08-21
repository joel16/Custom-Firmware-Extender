/*
 *	ctrl.c is part of HostCore
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
 *	Date Created:	2008-04-11
 */

#include <pspkernel.h>
#include <pspctrl.h>
#include <psputilsforkernel.h>
#include <pspsdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctrl.h"
#include "utils.h"
#include "syspatch.h"
#include "log.h"

unsigned int PSP_CTRL_CANCEL = PSP_CTRL_CIRCLE, lock = 0;
CtrlReadHandler handler = NULL;

void disablePadData( SceCtrlData * pad_data, int count )
{
	int i, intr = sceKernelCpuSuspendIntr();
	for(i = 0; i < count; i++)
	{
		pad_data[i].Buttons &= ~0xF0FFFF;
		pad_data[i].Lx = 128;
		pad_data[i].Ly = 128;
	}
	sceKernelCpuResumeIntr(intr);
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

int ( * sceCtrlReadBufferPositive_ori )( SceCtrlData * pad_data, int count );
int ( * sceCtrlPeekBufferPositive_ori )( SceCtrlData * pad_data, int count );
//int ( * sceCtrlReadBufferNegative_ori )( SceCtrlData * pad_data, int count );
//int ( * sceCtrlPeekBufferNegative_ori )( SceCtrlData * pad_data, int count );

int sceCtrlReadBufferPositive_new( SceCtrlData * pad_data, int count )
{
	int ret = sceCtrlReadBufferPositive_ori(pad_data, count);
	int k1 = pspSdkSetK1( 0 );
	if ( handler )
		handler( pad_data, count );
	if ( lock )
		disablePadData( pad_data, count );
	pspSdkSetK1( k1 );
	return ret;
}

/*int sceCtrlPeekBufferPositive_new( SceCtrlData * pad_data, int count )
{
	int ret;
	ret = sceCtrlPeekBufferPositive_ori( pad_data, count );
	int k1 = pspSdkSetK1( 0 );
	log( "ctrl patched\n" );
	pspSdkSetK1( k1 );
	return ret;
}

int sceCtrlReadBufferNegative_new( SceCtrlData * pad_data, int count )
{
	int ret = sceCtrlReadBufferNegative_ori( pad_data, count );
	int k1 = pspSdkSetK1( 0 );
	log( "ctrl patched\n" );
	pspSdkSetK1( k1 );
	return ret;
}

int sceCtrlPeekBufferNegative_new( SceCtrlData * pad_data, int count )
{
	int ret = sceCtrlPeekBufferNegative_ori(pad_data, count);
	int k1 = pspSdkSetK1( 0 );
	log( "ctrl patched\n" );
	pspSdkSetK1( k1 );
	return ret;
}*/

void ctrlLock()
{
	lock = 1;
}

void ctrlUnlock()
{
	lock = 0;
}

CtrlReadHandler setCtrlReadHandler( CtrlReadHandler read_handler )
{
	CtrlReadHandler previous = handler;
	handler = read_handler;
	return previous;
}

void initCtrl()
{
	unsigned int nid[2];
	getCtrlNids( nid );
	sceCtrlReadBufferPositive_ori = patchExport( "sceController_Service", "sceCtrl_driver", nid[0], sceCtrlReadBufferPositive_new );
	sceCtrlPeekBufferPositive_ori = ( void * )findProc( "sceController_Service", "sceCtrl_driver", nid[1] );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	unsigned int value = 1;
	getRegistryValue( "/CONFIG/SYSTEM/XMB", "button_assign", &value );
	if( value == 0 )
		PSP_CTRL_CANCEL = PSP_CTRL_CROSS;
}

unsigned int ctrlWaitKey( unsigned int key_mask, int timeout )
{
	int i = 0, count = timeout * 10;
	SceCtrlData pad_data;
	memset( &pad_data, 0, sizeof( SceCtrlData ) );
	while( !( pad_data.Buttons & key_mask ) && ( !timeout || i < count ) )
	{
		sceKernelDelayThread( 100000 );
		sceCtrlPeekBufferPositive_ori( &pad_data, 1 );
		i ++;
	}
	return pad_data.Buttons & key_mask;
}
