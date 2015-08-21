/*
 *	usbhost.c is part of HostCore
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
#include <pspinit.h>
#include <pspusb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syspatch.h"
#include "utils.h"
#include "log.h"

#define HOSTFSDRIVER_NAME	"USBHostFSDriver"
#define HOSTFSDRIVER_PID	(0x1C9)
#define USBHOSTFS_PRX		"ms0:/seplugins/cfe/usbhostfs.prx"

int startUsbHost()
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "USBHostFS" );
	if ( !pMod )
	{
		if ( loadStartModule( USBHOSTFS_PRX, 0, NULL ) < 0 )
		{
			log( "Error starting usbhostfs.prx\n" );
			return -1;
		}
	}
	int ret;
	if ( fw_version < FW_500 || sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_GAME )
	{
		ret = sceUsbStart( PSP_USBBUS_DRIVERNAME, 0, 0 );
		if ( ret != 0 )
		{
			log( "Error starting USB Bus driver (0x%08X)\n", ret );
			return -1;
		}
	}
	else
	{
		ret = sceUsbStart( PSP_USBBUS_DRIVERNAME, 0, 0 );
		if ( ret != 0 )
		{
			log( "Error starting USB Bus driver (0x%08X)\n", ret );
			return -1;
		}
	}

	ret = sceUsbStart( HOSTFSDRIVER_NAME, 0, 0 );
	if ( ret != 0 )
	{
		log( "Error starting USB Host driver (0x%08X)\n", ret );
		return -1;
	}
	ret = sceUsbActivate( HOSTFSDRIVER_PID );
	sceKernelDelayThread( 2000000 );
	return ret;
}

int stopUsbHost()
{
	int ret = sceUsbDeactivate( HOSTFSDRIVER_PID );
	if ( ret != 0 )
	{
		log( "Error Deactivate driver %08x (0x%08X)\n", HOSTFSDRIVER_PID, ret );
		//return -1;
	}
	ret = sceUsbStop( HOSTFSDRIVER_NAME, 0, 0 );
	if ( ret != 0 )
	{
		log( "Error stopping USB Host driver (0x%08X)\n", ret );
		//return -1;
	}
	if ( fw_version < FW_500 || sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_GAME )
	{
		sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		if ( ret != 0 )
		{
			log( "Error stopping USB Bus driver (0x%08X)\n", ret );
			//return -1;
		}
	}
	killModule( "USBHostFS" );
	return 0;
}
