/*
 *	wifihost.c is part of host2ms
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
 *	Date Created:	2008-04-06
 */

#include <pspkernel.h>
#include <pspinit.h>
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>
#include <pspnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wifihost.h"
#include "utils.h"
#include "syspatch.h"
#include "log.h"

static char drv_name[16];
static WifiCtrlOpts * ctrl_opts;
static int max_block_size = 2048;
static char password[12];
int init_key;
WifiCtrlOpts * ( * getNetCtrlOpts )( void );

#define FILE_ADDS				0x00001100

#define SEND_CMD( val )			*( sock_opts->buf ) = val;\
								if ( wifiSend( sock_opts, 4 ) != 4 )\
								{\
									log( "Error sending integer %08x\n", val );\
									return -1;\
								}
						
#define SEND_DATA( len )		if ( wifiSend( sock_opts, len ) != len )\
								{\
									log( "Error sending data with length %08x\n", len );\
									return -1;\
								}

#define RECV_DATA( len )		if ( wifiRecv( sock_opts, len ) != len )\
								{\
									log( "Error receiving data with length %08x\n", len );\
									return -1;\
								}

#define SEND_DAR( res, len )	res = wifiSend( sock_opts, len );\
								if ( res <= 0 )\
								{\
									log( "Error sending data with length %08x\n", len );\
									return -1;\
								}

#define RECV_DAR( res, len )	res = wifiRecv( sock_opts, len );\
								if ( res <= 0 )\
								{\
									log( "Error receiving data with length %08x\n", len );\
									return -1;\
								}
								
static SocketOpts * getIdleSock()
{
	int i = 0;
	while( 1 )
	{
		if ( ctrl_opts->sock[i].busy != 1 )
			return &ctrl_opts->sock[i];
		if ( i == 2 )
		{
			i = 0;
		}
		else i ++;
	}
	return NULL;
}

static int wifiSend( SocketOpts * sock_opts, int len )
{
	if ( sock_opts->sema < 0 )
		return -1;
	sock_opts->operation = S_SEND;
	sock_opts->length = len;
	//log( "send cmd %08x\n", *( sock_opts->buf ) );
	sceKernelWakeupThread( sock_opts->thid );
	sceKernelWaitSema( sock_opts->sema, 1, NULL );
	return sock_opts->res;
}

static int wifiRecv( SocketOpts * sock_opts, int len )
{
	if ( sock_opts->sema < 0 )
		return -1;
	sock_opts->operation = S_RECV;
	sock_opts->length = len;
	//log( "recv len %08x\n", len );
	sceKernelWakeupThread( sock_opts->thid );
	sceKernelWaitSema( sock_opts->sema, 1, NULL );
	return sock_opts->res;
}

void psw_encrypt( unsigned char * text, int textlen, unsigned char * key )
{
	int keylen, i, j, b;
	unsigned char a, lastkey;

	keylen = strlen( ( char * )key );
	lastkey = key[0];
	j = 0;
	for ( i = 0; i < textlen; i ++ )
	{
		a = key[j];
		text[i] = ( text[i] ^ a ) - j;
		b = lastkey % 8;
		text[i] = ( text[i] >> b ) + ( ( text[i] << ( 8 - b ) ) & 0xff );
		text[i] ^= lastkey + key[keylen - j - 1];
		lastkey = key[j];
		j ++;
		if ( j == keylen )
			j = 0;
	}
}

int wifiIoInit( PspIoDrvArg * arg )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	//wait hello from server
	SEND_CMD( NET_HOSTFS_CMD_HELLO );
	RECV_DATA( 4 );
	//init
	SEND_CMD( NET_HOSTFS_CMD_IOINIT );
	RECV_DATA( CHALLENGE_TEXT_LEN );
	
	psw_encrypt( ( unsigned char * )sock_opts->buf, CHALLENGE_TEXT_LEN, ( unsigned char * )password );
	
	SEND_DATA( CHALLENGE_TEXT_LEN );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoExit( PspIoDrvArg* arg )
{
	SocketOpts * sock_opts;
	int i = 0;
	for( i = 0; i < 4; i ++ )
	{
		while ( ctrl_opts->sock[i].busy == 1 )
			sceKernelDelayThread( 50000 );
		sock_opts = &ctrl_opts->sock[i];
		SEND_CMD( NET_HOSTFS_CMD_IOEXIT );
	}
	return 0;
}

int wifiIoOpen( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiOpenParams * opts = ( WifiOpenParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOOPEN;
	strcpy( opts->file, file );
	opts->fs_num = arg->fs_num;
	opts->flags = flags;
	opts->mode = mode;
	
	log( "open %s\n", opts->file );
	SEND_DATA( sizeof( WifiOpenParams ) );
	RECV_DATA( 4 );
	if ( *( sock_opts->buf ) < 0 )
	{
		sock_opts->busy = 0;
		return -1;
	}
	arg->arg = ( void * )( *( sock_opts->buf ) + FILE_ADDS );
	log( "fd %08x\n", *( sock_opts->buf ) );
	sock_opts->busy = 0;
	return 0;
}

int wifiIoClose( PspIoDrvFileArg * arg )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiCloseParams * opts = ( WifiCloseParams * )( sock_opts->buf );
	opts->cmd = NET_HOSTFS_CMD_IOCLOSE;
	opts->fd = ( int )arg->arg - FILE_ADDS;
	
	log( "close %08x\n", opts->fd );
	SEND_DATA( sizeof( WifiCloseParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoRead( PspIoDrvFileArg * arg, char * data, int len )
{
	if ( len < 0 )
		return -1;
	else if ( len == 0 )
		return 0;
	
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiReadParams * opts = ( WifiReadParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOREAD;
	opts->fd = ( int )arg->arg - FILE_ADDS;
	opts->len = len;
	//log( "read %08x size %08x\n", opts->fd, len );
	SEND_DATA( sizeof( WifiReadParams ) );
	RECV_DATA( 4 );
	
	int size = *( sock_opts->buf ), received = 0, blocksize, ret;
	//log( "total size %08x\n", size );
	if ( size > len || size <= 0 )
	{
		sock_opts->busy = 0;
		return -1;
	}
	//int * tmp = sock_opts->buf;
	while ( received < size )
	{
		blocksize = ( size - received ) >= max_block_size? max_block_size: ( size - received );
		//sock_opts->buf = ( int * )( data + received );
		RECV_DAR( ret, blocksize );
		memcpy( data + received, sock_opts->buf, ret );
		received += ret;
	}
	//sock_opts->buf = tmp;
	//log( "received %08x\n", received );
	sock_opts->busy = 0;
	return size;
}

int wifiIoWrite( PspIoDrvFileArg * arg, const char * data, int len )
{
	if ( len < 0 )
		return -1;
	else if ( len == 0 )
		return 0;
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiWriteParams * opts = ( WifiWriteParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOWRITE;
	opts->fd = ( int )arg->arg - FILE_ADDS;
	opts->len = len;
	
	SEND_DATA( sizeof( WifiWriteParams ) );
	
	int ret, sent = 0, blocksize;
	int * tmp = sock_opts->buf;
	while( sent < len )
	{
		blocksize = ( len - sent ) >= max_block_size? max_block_size: ( len - sent );
		//memcpy( ( sock_opts->buf ), data + sent, blocksize );
		sock_opts->buf = ( int * )( data + sent );
		SEND_DAR( ret, blocksize );
		sent += ret;
	}
	sock_opts->buf = tmp;
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

SceOff wifiIoLseek( PspIoDrvFileArg * arg, SceOff ofs, int whence )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiLseekParams * opts = ( WifiLseekParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOLSEEK;
	opts->fd = ( int )arg->arg - FILE_ADDS;
	opts->offset = ofs;
	//memcpy( &opts->offset, &ofs, 8 );
	opts->whence = whence;
	//log( "lseek %08x ofs %08x whence %08x\n", opts->fd, ofs, whence );
	SEND_DATA( sizeof( WifiLseekParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}
int wifiIoIoctl( PspIoDrvFileArg * arg, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen )
{
	return 0;
}

int wifiIoRemove( PspIoDrvFileArg * arg, const char * name )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiRemoveParams * opts = ( WifiRemoveParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOREMOVE;
	strcpy( opts->file, name );
	opts->fs_num = arg->fs_num;
	
	SEND_DATA( sizeof( WifiRemoveParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoMkdir( PspIoDrvFileArg * arg, const char * name, SceMode mode )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiMkdirParams * opts = ( WifiMkdirParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOMKDIR;
	strcpy( opts->dir, name );
	opts->fs_num = arg->fs_num;
	opts->mode = mode;
	
	SEND_DATA( sizeof( WifiMkdirParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoRmdir( PspIoDrvFileArg * arg, const char * name )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiRmdirParams * opts = ( WifiRmdirParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IORMDIR;
	strcpy( opts->dir, name );
	opts->fs_num = arg->fs_num;
	
	SEND_DATA( sizeof( WifiRmdirParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoDopen( PspIoDrvFileArg * arg, const char * dirname )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiDopenParams * opts = ( WifiDopenParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IODOPEN;
	strcpy( opts->dir, dirname );
	opts->fs_num = arg->fs_num;
	
	SEND_DATA( sizeof( WifiDopenParams ) );
	RECV_DATA( 4 );
	if ( *( sock_opts->buf ) < 0 )
	{
		sock_opts->busy = 0;
		return -1;
	}
	arg->arg = ( void * )( *( sock_opts->buf ) );
	log( "Dopened %08x\n", *( sock_opts->buf ) );
	sock_opts->busy = 0;
	return 0;
}

int wifiIoDclose( PspIoDrvFileArg * arg )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiDcloseParams * opts = ( WifiDcloseParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IODCLOSE;
	opts->fd = ( int )arg->arg;
	
	SEND_DATA( sizeof( WifiDcloseParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoDread( PspIoDrvFileArg * arg, SceIoDirent * dir )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiDreadParams * opts = ( WifiDreadParams * )( sock_opts->buf );
	WifiDreadResult * result = ( WifiDreadResult * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IODREAD;
	opts->fd = ( int )arg->arg;
	
	SEND_DATA( sizeof( WifiDreadParams ) );
	RECV_DATA( sizeof( WifiDreadResult ) );
	
	if ( result->res > 0 )
	{
		memcpy( dir, &result->entry, sizeof( SceIoDirent ) );
		//log( "dread %s\n", result->entry.d_name );
	}
	sock_opts->busy = 0;
	return result->res;
}

int wifiIoGetstat( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiGetstatParams * opts = ( WifiGetstatParams * )( sock_opts->buf );
	WifiGetstatResult * result = ( WifiGetstatResult * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOGETSTAT;
	strcpy( opts->file, file );
	opts->fs_num = arg->fs_num;
	
	SEND_DATA( sizeof( WifiGetstatParams ) );
	RECV_DATA( sizeof( WifiGetstatResult ) ); 
	
	if ( result->res >= 0 )
	{
		memcpy( stat, &result->stat, sizeof( SceIoStat ) );
	}
	sock_opts->busy = 0;
	return result->res;
}

int wifiIoChstat( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat, int bits )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiChstatParams * opts = ( WifiChstatParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IOCHSTAT;
	strcpy( opts->file, file );
	opts->fs_num = arg->fs_num;
	memcpy( &opts->stat, stat, sizeof( SceIoStat ) );
	opts->bits = bits;
	
	SEND_DATA( sizeof( WifiChstatParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoRename( PspIoDrvFileArg * arg, const char * oldname, const char * newname )
{
	SocketOpts * sock_opts = getIdleSock();
	sock_opts->busy = 1;
	WifiRenameParams * opts = ( WifiRenameParams * )( sock_opts->buf );
	
	opts->cmd = NET_HOSTFS_CMD_IORENAME;
	strcpy( opts->oldfile, oldname );
	strcpy( opts->newfile, newname );
	opts->fs_num = arg->fs_num;
	
	SEND_DATA( sizeof( WifiRenameParams ) );
	RECV_DATA( 4 );
	sock_opts->busy = 0;
	return *( sock_opts->buf );
}

int wifiIoChdir( PspIoDrvFileArg * arg, const char * dir )
{
	return 0;
}

int wifiIoMount( PspIoDrvFileArg * arg )
{
	return 0;
}

int wifiIoUmount( PspIoDrvFileArg * arg )
{
	return 0;
}


int wifiIoDevctl( PspIoDrvFileArg * arg, const char * devname, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen )
{
	return 0;
}

int wifiIoUnk21( PspIoDrvFileArg * arg )
{
	return 0;
}

PspIoDrvFuncs wifiIofuncs = 
{
	wifiIoInit,
	wifiIoExit,
	wifiIoOpen,
	wifiIoClose,
	wifiIoRead,
	wifiIoWrite,
	wifiIoLseek,
	wifiIoIoctl,
	wifiIoRemove,
	wifiIoMkdir,
	wifiIoRmdir,
	wifiIoDopen,
	wifiIoDclose,
	wifiIoDread,
	wifiIoGetstat,
	wifiIoChstat,
	wifiIoRename,
	wifiIoChdir,
    wifiIoMount,
    wifiIoUmount,
    wifiIoDevctl,
    wifiIoUnk21,
};

PspIoDrv wifihostfs_driver = 
{
	drv_name, 0x10, 0x800, "WifiHostFS", &wifiIofuncs
};

void disableFunc( unsigned int addr, unsigned int * ori )
{
	if ( ori )
	{
		ori[0] = _lw( addr );
		ori[1] = _lw( addr + 4 );
		ori[2] = _lw( addr + 8 );
	}
	_sw( 0x00001025, addr );
	_sw( 0x03e00008, addr + 4 );
	_sw( 0, addr + 8 );
}

void restoreFunc( unsigned int addr, unsigned int * ori )
{
	_sw( ori[0], addr );
	_sw( ori[1], addr + 4 );
	_sw( ori[2], addr + 8 );
}

unsigned int sceNetApctlDisconnect_ori[3],
			 sceNetApctlGetState_ori[3],
			 sceNetTerm_ori[3],
			 sceNetInetTerm_ori[3],
			 sceNetResolverTerm_ori[3],
			 sceNetApctlTerm_ori[3];

int stopWifiHost()
{
	if ( init_key == PSP_INIT_KEYCONFIG_GAME )
	{
		patchModule( "XceNetIfhandle_Service", 's' );
		patchModule( "XceNet_Library", 's' );
		patchModule( "XceNetInet_Library", 's' );
		patchModule( "XceNetResolver_Library", 's' );
		patchModule( "XceNetApctl_Library", 's' );
	}
	restoreFunc( findProc( "sceNet_Library", "sceNet", 0x281928A9 ), sceNetTerm_ori );
	restoreFunc( findProc( "sceNetInet_Library", "sceNetInet", 0xA9ED66B9 ), sceNetInetTerm_ori );
	restoreFunc( findProc( "sceNetResolver_Library", "sceNetResolver", 0x6138194A ), sceNetResolverTerm_ori );
	restoreFunc( findProc( "sceNetApctl_Library", "sceNetApctl",0xB3EDD0EC ), sceNetApctlTerm_ori );
	
	sceIoDelDrv( wifihostfs_driver.name );
	if ( ctrl_opts->thid >= 0 )
		sceKernelWakeupThread( ctrl_opts->thid );
	while ( ctrl_opts->inited != -1 )
		sceKernelDelayThread( 1000000 );
	killModule( "wifiuser" );
	killModule( "sceNetResolver_Library" );
	killModule( "sceNetInet_Library" );
	killModule( "sceNetApctl_Library" );
	killModule( "sceNet_Library" );
	killModule( "sceNetInterface_Service" );
	return 0;
}

int startWifiHost( const char * name, HostCoreConf * config )
{	
	init_key = sceKernelInitKeyConfig();
	int pid = 2;
	char * argv[4];
	
	strcpy( drv_name, name? name: "host" );
	max_block_size = atoi( config->blocksize );
	
	memset( password, 0, 12 );
	if ( config->password[0] != 0 )
	{
		strncpy( password, config->password, 8 );
		password[8] = 0;
	}
	log( "password %s\n", password );
	
	argv[0] = strlen( config->ip )? config->ip: "noaddress";
	argv[1] = strlen( config->port )? config->port: "7513";
	argv[2] = strlen( config->entry )? config->entry: "1";
	argv[3] = strlen( config->blocksize )? config->blocksize: "2048";
	
	int ret = loadStartModule( "flash0:/kd/ifhandle.prx", 0, NULL );
	if ( ret < 0 )
	{
		log( "Error on starting ifhandle.prx %08x\n", ret );
		return -1;
	}
	if ( init_key == PSP_INIT_KEYCONFIG_GAME )
	{
		wifiModulesPatch1();
		pid = 4;
	}
	ret = loadStartModulePartition( pid, "flash0:/kd/pspnet.prx", 0, NULL );
	if ( ret < 0 )
	{
		log( "Error on starting pspnet.prx %08x\n", ret );
		return -1;
	}
	ret = loadStartModulePartition( pid, "flash0:/kd/pspnet_inet.prx", 0, NULL );
	if ( ret < 0 )
	{
		log( "Error on starting pspnet_inet.prx %08x\n", ret );
		return -1;
	}
	ret = loadStartModulePartition( pid, "flash0:/kd/pspnet_apctl.prx", 0, NULL );
	if ( ret < 0 )
	{
		log( "Error on starting pspnet_apctl.prx %08x\n", ret );
		return -1;
	}
	ret = loadStartModulePartition( pid, "flash0:/kd/pspnet_resolver.prx", 0, NULL );
	if ( ret < 0 )
	{
		log( "Error on starting pspnet_resolver.prx %08x\n", ret );
		return -1;
	}
	
	if ( init_key == PSP_INIT_KEYCONFIG_GAME )
	{
		wifiModulesPatch2();
	}
	
	ret = loadStartModulePartition( pid, "ms0:/seplugins/cfe/wifiuser.prx", 4, argv );
	if ( ret < 0 )
	{
		log( "Error on starting wifiuser.prx\n" );
		return -1;
	}
	
	getNetCtrlOpts = ( void * )findProc( "wifiuser", "WifiUserLib", 0x43da9872 );
	ctrl_opts = getNetCtrlOpts();
	
	while ( !ctrl_opts->inited )
		sceKernelDelayThread( 500000 );
	
	if ( init_key == PSP_INIT_KEYCONFIG_GAME )
	{
		wifiModulesPatch3();
	}
	
	if ( ctrl_opts->inited < 0 )
	{
		log( "Error on init wifiuser (%08x)\n", ctrl_opts->inited );
		stopWifiHost();
		return -1;
	}
	
	disableFunc( findProc( "sceNet_Library", "sceNet", 0x39AF39A6 ), NULL );
	disableFunc( findProc( "sceNetInet_Library", "sceNetInet", 0x17943399 ), NULL );
	disableFunc( findProc( "sceNetResolver_Library", "sceNetResolver", 0xF3370E61 ), NULL );
	disableFunc( findProc( "sceNetApctl_Library", "sceNetApctl", 0xE2F91F9B ), NULL );
	
	disableFunc( findProc( "sceNet_Library", "sceNet", 0x281928A9 ), sceNetTerm_ori );
	disableFunc( findProc( "sceNetInet_Library", "sceNetInet", 0xA9ED66B9 ), sceNetInetTerm_ori );
	disableFunc( findProc( "sceNetResolver_Library", "sceNetResolver", 0x6138194A ), sceNetResolverTerm_ori );
	disableFunc( findProc( "sceNetApctl_Library", "sceNetApctl",0xB3EDD0EC ), sceNetApctlTerm_ori );
	
	if ( init_key == PSP_INIT_KEYCONFIG_GAME )
	{
		patchModule( "sceNetIfhandle_Service", 'X' );
		patchModule( "sceNet_Library", 'X' );
		patchModule( "sceNetInet_Library", 'X' );
		patchModule( "sceNetResolver_Library", 'X' );
		patchModule( "sceNetApctl_Library", 'X' );
	}
	
	sceIoDelDrv( wifihostfs_driver.name );
	ret = sceIoAddDrv( &wifihostfs_driver );
	if ( ret < 0 )
	{
		log( "Error on adding wifihostfs drv\n" );
		return -1;
	}
	log( "drv %s added\n", wifihostfs_driver.name );
	
	return 0;
}
