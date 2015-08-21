/*
 *	syspatch.c is part of HostCore
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
 *	Date Created:	2008-05-15
 */

#include <pspkernel.h>
#include <pspsdk.h>
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int fw_version = FW_371;
int model = PSP_MODEL_STANDARD;

int ( * getDevkitVersion )( void );

int initPatches( void )
{
	getDevkitVersion = ( void * )findProc( "sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A );
	fw_version = getDevkitVersion();
	model = sceKernelGetModel();
	//Disable slim specific patches
	model = PSP_MODEL_STANDARD;
	return fw_version;
}

unsigned int getFindDriverAddr( void )
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceIOFileManager" );
	unsigned int addr = 0;
	if ( !pMod )
		return 0;
	if ( fw_version == FW_371 )
		addr = pMod->text_addr + 0x00002844;
	else if ( fw_version == FW_380 || fw_version == FW_390 )
		addr = pMod->text_addr + 0x00002808;
	else if ( fw_version == FW_401 )
		addr = pMod->text_addr + 0x000027EC;
	else if ( fw_version == FW_500 )
		addr = pMod->text_addr + 0x00002838;
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
	{
		addr = pMod->text_addr + 0x00002A4C;
	}
	return addr;
}

void getCtrlNids( unsigned int * nid )
{
	if ( fw_version == FW_371 )
	{
		nid[0] = 0x454455ac; //sceCtrlReadBufferPositive
		nid[1] = 0xc4aad55f; //sceCtrlPeekBufferPositive
	}
	else if ( fw_version == FW_380 || fw_version == FW_390 )
	{
		nid[0] = 0xad0510f6;
		nid[1] = 0xd65d4e9a;
	}
	else if ( fw_version == FW_401 )
	{
		nid[0] = 0xBA664B5E;
		nid[1] = 0x591B3F36;
	}
	else if ( fw_version == FW_500 )
	{
		nid[0] = 0x919215D7;
		nid[1] = 0x6B247CCE;
	}
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
	{
		nid[0] = 0x1F803938;
		nid[1] = 0x3A622550;
	}
}

void getUtilsNids( unsigned int * nid )
{
	if ( fw_version == FW_371 )
	{
		nid[0] = 0xa3d5e142; //sceKernelExitVSHVSH
		nid[1] = 0xd9739b89; //sceKernelUnregisterExitCallback
		nid[2] = 0x659188e1; //sceKernelCheckExitCallback
		nid[3] = 0x49C5B9E1; //sceKernelLoadModuleForLoadExecVSHMs2
		nid[4] = 0xa1a78C58; //sceKernelLoadModuleForLoadExecVSHDisc
	}
	else if ( fw_version == FW_380 || fw_version == FW_390 )
	{
		nid[0] = 0x62879ad8;
		nid[1] = 0xf1c99c38;
		nid[2] = 0x753ef37c;
		nid[3] = 0x42ED1407;
		nid[4] = 0xc8f0090d;
	}
	else if ( fw_version == FW_401 )
	{
		nid[0] = 0xCA8011A2;
		nid[1] = 0x5AF87B62;
		nid[2] = 0x6274D0D5;
		nid[3] = 0x313F2757;
		nid[4] = 0x83B28C87;
	}
	else if ( fw_version == FW_500 )
	{
		nid[0] = 0x94A1C627;
		nid[1] = 0x71F9FB1B;
		nid[2] = 0x2E96EDF8;
		nid[3] = 0xB8E49712;
		nid[4] = 0x7C8A2B62;
	}
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
	{
		nid[0] = 0x08F7166C;
		nid[1] = 0x24114598;
		nid[2] = 0xB57D0DEC;
		nid[3] = 0x7BD53193;
		nid[4] = 0xCE0A74A5;
	}
}

unsigned int getKillMutexNid()
{
	return 0xf8170fbe;
}

void getDisplayNids( unsigned int * nid )
{
	if ( fw_version == FW_371 )
	{
		nid[0] = 0xe56b11ba;
		nid[1] = 0x7fba941a;
	}
	else if ( fw_version == FW_380 || fw_version == FW_390 )
	{
		nid[0] = 0x3749cda0;
		nid[1] = 0xc89e1f1d;
	}
	else if ( fw_version == FW_401 )
	{
		nid[0] = 0xC28EFAA7;
		nid[1] = 0xC922270C;
	}
	else if ( fw_version == FW_500 )
	{
		nid[0] = 0xD8D2FD35;
		nid[1] = 0xFBDA7A1E;
	}
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
	{
		nid[0] = 0x1F803938;
		nid[1] = 0x3A622550;
	}
}

void patchMemPartitionInfo()
{
	if ( model == PSP_MODEL_STANDARD )
		sceKernelSetDdrMemoryProtection( ( void * )0x88300000, 0x00100000, 0xf );
	else sceKernelSetDdrMemoryProtection( ( void * )0x88600000, 0x00200000, 0xf );
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceSystemMemoryManager" );
	// 0x02001021 move $v0 $s0
	int offset = 0x00001304;
	if ( fw_version == FW_371 || fw_version == FW_380 || fw_version == FW_390 )
	{
		offset = 0x00001304; //for 3.71, 3.80, 3.90
	}
	else if ( fw_version == FW_401 )
	{
		offset = 0x00003A68; //for 4.01
	}
	else if ( fw_version == FW_500 )
	{
		offset = 0x00003AA8; //for 5.00
	}
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
	{
		offset = 0x00004184;
	}
	_sw( 0x02001021, pMod->text_addr + offset );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	PspSysmemPartitionInfo info;
	memset( &info, 0, sizeof( PspSysmemPartitionInfo ) );
	info.size = sizeof( PspSysmemPartitionInfo );
	PspSysmemPartitionInfo * p_info = ( PspSysmemPartitionInfo * )sceKernelQueryMemoryPartitionInfo( 4, &info );
	if ( model == PSP_MODEL_STANDARD )
		p_info->startaddr = 0x08300000;
	else p_info->startaddr = 0x08600000;
	p_info->attr = 0xf;
	//restore
	_sw( 0x00001021, pMod->text_addr + offset );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

typedef struct PatchSav
{
	unsigned int addr;
	unsigned int val;
} PatchSav;

PatchSav LoadExecVSHCommon_ori[2];

void restoreLoadExecVSHCommon()
{
	_sw( LoadExecVSHCommon_ori[0].val, LoadExecVSHCommon_ori[0].addr );
	_sw( LoadExecVSHCommon_ori[1].val, LoadExecVSHCommon_ori[1].addr );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

void * patchLoadExecVSHCommon( void * func )
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceLoadExec" );
	if ( fw_version == FW_371 )
		LoadExecVSHCommon_ori[0].addr = pMod->text_addr + 0x0000121c; //same in standare/slim
	else if ( fw_version == FW_380 || fw_version == FW_390 )
		LoadExecVSHCommon_ori[0].addr = pMod->text_addr + 0x000014cc; //same in standare/slim
	else if ( fw_version == FW_401 )
		LoadExecVSHCommon_ori[0].addr = pMod->text_addr + 0x00001E1C; //same in standare/slim
	else if ( fw_version == FW_500 )
		LoadExecVSHCommon_ori[0].addr = pMod->text_addr + 0x00001E58; //verified in phat
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
		LoadExecVSHCommon_ori[0].addr = pMod->text_addr + 0x00001F3C;
	LoadExecVSHCommon_ori[1].addr = LoadExecVSHCommon_ori[0].addr + 4;
	LoadExecVSHCommon_ori[0].val = _lw( LoadExecVSHCommon_ori[0].addr );
	LoadExecVSHCommon_ori[1].val = _lw( LoadExecVSHCommon_ori[1].addr );
	MAKE_JUMP( LoadExecVSHCommon_ori[0].addr, func );
	_sw( NOP, LoadExecVSHCommon_ori[1].addr );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	return ( void * )LoadExecVSHCommon_ori[0].addr;
}

unsigned int modulemgr_offset = 0, threadman_offset = 0;

void wifiModulesPatch1()
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceThreadManager" );
	//a0 = 4, change partition id to 4
	if ( fw_version == FW_371 )
		threadman_offset = 0x00010B30;
	else if ( fw_version == FW_380 || fw_version == FW_390 )
		threadman_offset = 0x00010CB8;
	else if ( fw_version == FW_401 )
		threadman_offset = 0x00012154;
	else if ( fw_version == FW_500 )
		threadman_offset = 0x000121E0;
	_sw( 0x34040004, pMod->text_addr + threadman_offset );

	pMod = ( tSceModule * )sceKernelFindModuleByName( "sceModuleManager" );
	//a3 stack size 0x40000 -> 0x10000
	if ( fw_version == FW_371 )
		modulemgr_offset = 0x000076A0;
	else if ( fw_version == FW_380 || fw_version == FW_390 )
		modulemgr_offset = 0x00007C9C;
	else if ( fw_version == FW_401 )
		modulemgr_offset = 0x00007C50;
	else if ( fw_version == FW_500 )
		modulemgr_offset = 0x00007C84;
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
		modulemgr_offset = 0x00007C84;
	_sw( 0x3C070001, pMod->text_addr + modulemgr_offset );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

void wifiModulesPatch2()
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceNetInterface_Service" );
	//a2 partid = 4 of ifhandle
	_sw( 0x34050004, pMod->text_addr + 0x00001440 );  //for 3.71, 3.80, 3.90, 4.01, 5.00

	pMod = ( tSceModule * )sceKernelFindModuleByName( "sceNet_Library" );
	unsigned int net_offset = 0;
	if ( fw_version == FW_371 || fw_version == FW_380 || fw_version == FW_390 )
		net_offset = 0x00001800;
	else if ( fw_version == FW_401 )
		net_offset = 0x00002320;
	else if ( fw_version == FW_500 )
		net_offset = 0x00002348;
	else if ( fw_version == FW_620 || fw_version == FW_635 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661 )
		net_offset = 0x00002348;
	_sw( 0x34020002, pMod->text_addr + net_offset );
	_sw( 0xAFA20000, pMod->text_addr + net_offset + 0x4 );
	_sw( 0x3C020000, pMod->text_addr + net_offset + 0xC );
	
	pMod = ( tSceModule * )sceKernelFindModuleByName( "sceModuleManager" );
	//a3 stack size 0x10000 -> 0x4000
	_sw( 0x34074000, pMod->text_addr + modulemgr_offset );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

void wifiModulesPatch3()
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceModuleManager" );
	//restore
	_sw( 0x02403821, pMod->text_addr + modulemgr_offset );

	pMod = ( tSceModule * )sceKernelFindModuleByName( "sceThreadManager" );
	//restore
	_sw( 0x02402021, pMod->text_addr + threadman_offset );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

