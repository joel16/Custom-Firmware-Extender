/*
 *	main.c is part of launcher
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
 *	Date Created:	2008-05-09
 */

#include <pspuser.h>
#include <pspdebug.h>
#include <pspsdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO( "cfe_launcher", 0, 1, 1 );
PSP_MAIN_THREAD_ATTR( THREAD_ATTR_USER );

int buildArgs( char * args, int argc, char ** argv )
{
	int len = 0, i;
	for ( i = 0; i < argc; i ++ )
	{
		strcpy( &args[len], argv[i] );
		len += strlen( argv[i] ) + 1;
	}
	return len;
}

int loadStartModulePartition( int pid, char * file, int argc, char ** argv )
{
	SceKernelLMOption option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = pid;
	option.mpiddata = pid;
	option.position = 0;
	option.access = 1;
	int modid = sceKernelLoadModule( file, 0, &option );
	if ( modid >= 0 )
	{
		char args[512];
		int status, len = 0;
		strcpy( args, file );
		len += strlen( file ) + 1;
		len += buildArgs( &args[len], argc, argv );
		int ret = sceKernelStartModule( modid, len, ( void * )args, &status, NULL );
		if ( ret < 0 )
		{
			ret = sceKernelUnloadModule( modid );
			return ret;
		}
	}
	return modid;
}

int main( int argc, char ** argv )
{
	char * mode = ( char * )0x09000000;
	char * exec = ( char * )0x09000010;
	strcpy( mode, argv[1] );
	strcpy( exec, argv[2] );
	loadStartModulePartition( 1, "ms0:/seplugins/cfe/cfe_redirect.prx", 0, NULL );
	int status;
	return sceKernelStopUnloadSelfModule( 0, NULL, &status, NULL );
}
