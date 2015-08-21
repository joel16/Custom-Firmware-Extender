/*
 *	main.cpp is part of wifiuser
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
 *	Date Created:	2008-04-07
 */

#include <pspuser.h>
#include <psputility_netmodules.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <pspnet_apctl.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../log.h"
#include "../wifihost.h"

PSP_MODULE_INFO( "wifiuser", 0, 1, 0 );
PSP_MAIN_THREAD_ATTR( PSP_THREAD_ATTR_USER );

static WifiCtrlOpts ctrl_opts;
static int block_size = 2048;

int sock_thread( SceSize args, void *argp )
{
	unsigned char buffer[block_size], running = 1;
	SocketOpts * opts = ( SocketOpts * )( *( unsigned int * )argp );
	opts->buf = ( int * )buffer;
	opts->sema = sceKernelCreateSema( "sock_sema", 0, 0, 1, NULL );
	while( running )
	{
		sceKernelSleepThread();
		switch ( opts->operation )
		{
			case S_SEND:
				opts->res = sceNetInetSend( opts->server, opts->buf, opts->length, 0 );
				log( "sent %d res %08x\n", opts->length, opts->res );
				break;
			case S_RECV:
				opts->res = sceNetInetRecv( opts->server, opts->buf, opts->length, 0 );
				break;
			case S_TERM:
				running = 0;
				break;
		}
		sceKernelSignalSema( opts->sema, 1 );
	}
	sceKernelDeleteSema( opts->sema );
	opts->sema = -1;
	opts->thid = -1;
	opts->buf = NULL;
	return sceKernelExitDeleteThread( 0 );
}

int connectApctl( int config )
{
	int stat_last = -1;
	int ret = sceNetApctlConnect( config );
	while ( ret >= 0 )
	{
		int stat;
		ret = sceNetApctlGetState( &stat );
		if ( ret < 0 )
			break;
		if ( stat > stat_last )
		{
			log( "connection state %d\n", stat );
			stat_last = stat;
		}
		if ( stat < stat_last )
		{
			ret = -1;
			break;
		}
		if ( stat == 4 )
			break;
		sceKernelDelayThread( 50000 );
	}
	if ( ret < 0 )
	{
		log( "Error connecting to entry %d (0x%08x)\n", config, ret );
		return ret;
	}
	return 0;
}

int disconnectApctl()
{
	int stat = 1;
	int ret = sceNetApctlDisconnect();
	while( stat && ret >= 0 )
	{
		ret = sceNetApctlGetState( &stat );
		sceKernelDelayThread( 50000 );
	}
	return 0;
}

int connectSocket( const char * ip_addr, unsigned short port )
{
	struct sockaddr_in addr;
	int opt = 1;
	
	addr.sin_family = AF_INET;
    addr.sin_port = htons( port ); 
    addr.sin_addr.s_addr = sceNetInetInetAddr( ip_addr );
	
	int i = 0;
	for( i = 0; i < 4; i ++ )
	{
		ctrl_opts.sock[i].server = sceNetInetSocket( PF_INET, SOCK_STREAM, 0 );
		if ( ctrl_opts.sock[i].server < 0 )
		{
			log( "Error create sock connection\n" );
			return -1;
		}
		int ret = sceNetInetSetsockopt( ctrl_opts.sock[i].server, SOL_TCP, TCP_NODELAY, &opt, sizeof( opt ) );
		if ( ret != 0 )
		{
			log( "Error setting sock option\n" );
			return -1;
		}

		ret = sceNetInetConnect( ctrl_opts.sock[i].server, ( struct sockaddr * )&addr, sizeof( addr ) );
		if ( ret < 0 )
		{
			log( "Error connect to sock %08x\n", ctrl_opts.sock[i].server );
			return -1;
		}
		ctrl_opts.sock[i].thid = sceKernelCreateThread( "sock_thread", sock_thread, 0x20, 0x2000, PSP_THREAD_ATTR_USBWLAN, NULL );
		if ( ctrl_opts.sock[i].thid < 0 )
		{
			log( "Error creating thread\n" );
			return -1;
		}
		unsigned int addr = ( unsigned int )&ctrl_opts.sock[i];
		sceKernelStartThread( ctrl_opts.sock[i].thid, 4, &addr );
	}
	
	return 0;
}

int disconnectSocket()
{
	int i;
	for ( i = 0; i < 4; i ++ )
	{
		if ( ctrl_opts.sock[i].thid >= 0 )
		{
			ctrl_opts.sock[i].operation = S_TERM;
			sceKernelWakeupThread( ctrl_opts.sock[i].thid );
		}
		sceNetInetClose( ctrl_opts.sock[i].server );
		ctrl_opts.sock[i].server = -1;
	}
	return 0;
}

WifiCtrlOpts * getNetCtrlOpts()
{
	return &ctrl_opts;
}

int parseArgs( char ** argv, int args, char * argp )
{
	int len = 0, i;
	for ( i = 0; len < args; i ++ )
	{
		argv[i] = &argp[len];
		len += strlen( &argp[len] ) + 1;
	}
	return i;
}

int main_thread( SceSize args, void *argp )
{
	char ip[16], * argv[5];
	int port, entry;
	parseArgs( argv, args, ( char * )argp );
	//sceUtilityGetNetParam
	strcpy( ip, argv[1] );
	port = atoi( argv[2] );
	entry = atoi( argv[3] );
	block_size = atoi( argv[4] );
	log( "%s:%d entry %d\n", ip, port, entry );
	
	/*ctrl_opts.inited = sceUtilityLoadNetModule( PSP_NET_MODULE_COMMON );
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error loading Net modules (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}
	ctrl_opts.inited = sceUtilityLoadNetModule( PSP_NET_MODULE_INET );
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error loading iNet module (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}*/
	ctrl_opts.inited = sceNetInit( 0x10000, 0x20, 0x1000, 0x20, 0x1000 );
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error Initing pspnet (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}
	ctrl_opts.inited = sceNetInetInit();
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error initing Inet (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}
	ctrl_opts.inited = sceNetResolverInit();
	if( ctrl_opts.inited != 0 )
	{
		log( "Error initing Resolver (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}
	ctrl_opts.inited = sceNetApctlInit( 0x1400, 0x42 );
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error initing Apctl (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}
	log( "pspnet init OK!\n" );
	
	ctrl_opts.inited = connectApctl( entry );
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error connecting Apctl (0x%08x)\n", ctrl_opts.inited );
		goto net_term;
	}
	
	ctrl_opts.inited = connectSocket( ip, ( unsigned short )port );
	if ( ctrl_opts.inited != 0 )
	{
		log( "Error connecting Socket\n" );
		goto net_term;
	}
	ctrl_opts.inited = 1;
	
	sceKernelSleepThread();
	
net_term:
	log( "stopping wifi...\n" );
	disconnectSocket();
	disconnectApctl();
	sceNetApctlTerm();
	sceNetResolverTerm();
	sceNetInetTerm();
	sceNetTerm();
	//sceUtilityUnloadNetModule( PSP_NET_MODULE_INET );
	//sceUtilityUnloadNetModule( PSP_NET_MODULE_COMMON );
	ctrl_opts.thid = -1;
	ctrl_opts.inited = -1;
	return sceKernelExitDeleteThread( 0 );
}

int module_start( SceSize args, void *argp )
{
	memset( &ctrl_opts, 0, sizeof( WifiCtrlOpts ) );
	ctrl_opts.thid = sceKernelCreateThread( "wifhost_thread", main_thread, 0x20, 0x2000, PSP_THREAD_ATTR_USBWLAN, NULL );
	if( ctrl_opts.thid >= 0 )
		sceKernelStartThread( ctrl_opts.thid, args, argp );
	return 0;
}

int module_stop( SceSize args, void *argp )
{
	return 0;
}
