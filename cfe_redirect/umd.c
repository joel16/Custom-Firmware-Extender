/*
 *	noumd.c is part of host2ms
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
 *	Date Created:	2008-04-03
 */

#include <pspkernel.h>
#include <pspumd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "log.h"
#include "umd.h"

char * umd_executable[] =
{
	"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN",
	"disc0:/PSP_GAME/SYSDIR/BOOT.BIN",
};

int mode = 0, executable = 0;

void initUmdImageDriver()
{
	CfwConfig config;
	memset( &config, 0, sizeof( CfwConfig ) );
	getCfwConfig( &config );
	mode = config.umdmode;
	executable = config.executebootbin;
	log( "umdmode %d exec %d\n", mode, executable );
	switch ( mode )
	{
		case MODE_MARCH33:
		{
			sceIoDelDrv( "isofs" );
			killModule( "sceIsofs_driver" );
			if ( !findProc( "pspMarch33_Driver", NULL, 0xcee8593c ) )
			{
				sceIoDelDrv( "umd" );
			}
			killModule( "pspMarch33_Driver" );
			killSema( "Semaphore", "MediaManSema" );
			break;
		}
		case MODE_NP9660:
		{
			sceIoDelDrv( "isofs" );
			killModule( "sceIsofs_driver" );
			sceIoDelDrv( "umd" );
			killThread( "Thread", "SceNpUmdMount" );
			killEventHandler( "SceUmdMedia" );
			killModule( "sceNp9660_driver" );
			killEventFlag( "EventFlag", "SceUmdManState" );
			killEventFlag( "EventFlag", "SceMediaManUser" );
			break;
		}
	}
}

void mountUmdImage( char * file )
{
	switch ( mode )
	{
		case MODE_UMD:
		{
			mountUmdFromFile( file, 0, 0 );
			break;
		}
		case MODE_OE_LEGACY:
		{
			mountUmdFromFile( file, 1, 1 );
			break;
		}
		case MODE_MARCH33:
		{
			setInitApitype( 0x120 );
			setInitFileName( file );
			setUmdFile( file );
			loadStartModule( "flash0:/kd/march33.prx", 0, NULL );
			loadStartModule( "flash0:/kd/isofs.prx", 0, NULL );
			break;
		}
		case MODE_NP9660:
		{
			setInitApitype( 0x120 );
			setInitFileName( file );
			setUmdFile( file );
			loadStartModule( "flash0:/kd/mgr.prx", 0, NULL );
			loadStartModule( "flash0:/kd/npdrm.prx", 0, NULL );
			loadStartModule( "flash0:/kd/galaxy.prx", 0, NULL );
			loadStartModule( "flash0:/kd/np9660.prx", 0, NULL );
			loadStartModule( "flash0:/kd/isofs.prx", 0, NULL );
			break;
		}
	}
	int stat = sceUmdCheckMedium();
	if ( stat == 0 )
	{
		log( "no media\n" );
		sceUmdWaitDriveStat( UMD_WAITFORDISC );
	}
	sceUmdActivate( 1, "disc0:" );
	sceUmdWaitDriveStat( UMD_WAITFORINIT );
}

int launchUmdImage()
{
	int modid = sceKernelLoadModuleForLoadExecVSHDisc( umd_executable[executable], 0, NULL );
	log( "load %s modid %08x\n", umd_executable[executable], modid );
	if ( modid >= 0 )
	{
		int status, len = strlen( umd_executable[executable] ) + 1;
		modid = sceKernelStartModule( modid, len, ( void * )umd_executable[executable], &status, NULL );
		log( "start ret %08x\n", modid );
	}
	return modid;
}
