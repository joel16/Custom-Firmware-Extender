/*
 *	main.c is part of host2ms
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
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>
#include <psploadexec_kernel.h>
#include <psploadcore.h>
#include <pspdisplay.h>
#include <pspinit.h>
#include <pspctrl.h>
#include <pspumd.h>
#include <pspsdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "syspatch.h"
#include "ctrl.h"
#include "tinyui.h"
#include "utils.h"
#include "usbhost.h"
#include "wifihost.h"
#include "umd.h"
#include "log.h"

PSP_MODULE_INFO("cfe_redirect", 0x1000, 1, 4);
PSP_MAIN_THREAD_ATTR(0);

#define MIN( a, b )			( a < b? a: b )
#define MODE_FILE			"ms0:/seplugins/cfe/mark.txt"
#define LAUNCHER "ms0:/seplugins/cfe/EBOOT.PBP"

enum {
	D_USB = 1,
	D_WIFI = 2,
} HostDevice;

typedef struct HostCoreOpt {
	char name[16];
	char val[16];
} HostCoreOpt;

typedef struct DRecord {
	void * ms_arg;
	PspIoDrvFileArg host_arg;
	unsigned char read_end;
} DRecord;

typedef struct SceFatMsDirentPrivate {
	unsigned int attr;
	char short_name[0xC];
	unsigned int unk;
	char long_name[0x100];
} SceFatMsDirentPrivate;

const char * mode_str[] =
{
	"OFF",
	"USB",
	"WIF",
};

const char * icons[] =
{
	"ms0:/HostCore/resources/off.res",
	"ms0:/HostCore/resources/usb.res",
	"ms0:/HostCore/resources/wifi.res",
};

static PspIoDrv * fatms_drv;
static PspIoDrv * hostfs_drv;
static PspIoDrvArg * host_drv = NULL;
static void ** records = NULL;
static DRecord * d_records = NULL;
static int heap_id = -1, count = 0, d_count = 0, thid = -1, pressed = 0;
static unsigned int hot_key = 0;
StartModuleHandler previous = NULL;
CtrlReadHandler ctrl_previous = NULL;
HostCoreConf hc_conf;
int init_key;
static int host_mode = 0;
static char exec[128];

static int isRedirected( PspIoDrvFileArg * arg )
{
	if ( !records )
		return -1;
	int i;
	for( i = 0; i < count; i ++ )
	{
		if( arg->arg == records[i] )
			return i;
	}
	return -1;
}

static int isCombinated( PspIoDrvFileArg * arg )
{
	if ( !d_records )
		return -1;
	int i;
	for( i = 0; i < d_count; i ++ )
	{
		if( arg->arg == d_records[i].ms_arg )
			return i;
	}
	return -1;
}

static char * getShortName( char * des, const char * src )
{
	int i, j, len = strlen( src );
	for ( i = 0; i < MIN( len, 6 ); i ++ )
	{
		des[i] = toupper( src[i] );
	}
	if ( len > 0xC )
	{
		des[i ++] = '~';
		des[i ++] = '1';
		j = len - 4;
	}
	else j = i;
	for( ; j < len; j ++, i ++ )
	{
		des[i] = toupper( src[i] );
	}
	return des;
}

static SceFatMsDirentPrivate * setDirentPrivate( SceFatMsDirentPrivate * private, SceIoDirent * dir )
{
	if ( !private )
		return NULL;
	private->attr = 0x414;
	getShortName( private->short_name, dir->d_name );
	private->unk = 0;
	strcpy( private->long_name, dir->d_name );
	return private;
}

int ( * hostfsIoOpen )( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode );
int ( * msIoOpen )( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode );
int ( * msIoClose )( PspIoDrvFileArg * arg );
int ( * msIoRead )( PspIoDrvFileArg * arg, char * data, int len );
SceOff ( * msIoLseek )( PspIoDrvFileArg * arg, SceOff ofs, int whence );
int ( * msIoIoctl )( PspIoDrvFileArg * arg, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen );
int ( * msIoDopen )( PspIoDrvFileArg * arg, const char * dirname );
int ( * msIoDclose )( PspIoDrvFileArg * arg );
int ( * msIoDread )( PspIoDrvFileArg * arg, SceIoDirent * dir );
int ( * msIoGetstat )( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat );
int ( * msIoChdir )( PspIoDrvFileArg * arg, const char * dir );
 
int hostfsIoOpen_new( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode )
{
	host_drv = arg->drv;
	return hostfsIoOpen( arg, file, flags, mode );
}

int msIoOpen_new( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode )
{
	int ret = msIoOpen( arg, file, flags, mode );
	if ( strstr( file , "/SAVEDATA" ) )
		return ret;
	if ( ret < 0 && ( flags & PSP_O_WRONLY ) == 0 )
	{
		while( host_mode && !host_drv )
		{
			log( "waiting for host %s\n", file );
			sceKernelDelayThread( 1000000 );
		}
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		ret = hostfsIoOpen( arg, file, flags, mode );
		arg->drv = drv;
		log( "try to open %s at host0 %08x\n", file, ( unsigned int )arg->arg );
		if ( ret >= 0 )
		{
			records[count ++] = arg->arg;
			if ( count % 32 == 0 )
			{
				void ** tmp = sceKernelAllocHeapMemory( heap_id, 4 * ( count + 32 ) );
				memcpy( tmp, records, 4 * count );
				sceKernelFreeHeapMemory( heap_id, records );
				records = tmp;
			}
		}
	}
	return ret;
}

int msIoClose_new( PspIoDrvFileArg * arg )
{
	int num = isRedirected( arg );
	if ( num >= 0 )
	{
		log( "close %08x at host0\n", ( unsigned int )arg->arg );
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		int ret = hostfs_drv->funcs->IoClose( arg );
		arg->drv = drv;
		count --;
		memcpy( &records[num], &records[num + 1], 4 * ( count - num ) );
		return ret;
	}
	return msIoClose( arg );
}

int msIoRead_new( PspIoDrvFileArg * arg, char * data, int len )
{
	int num = isRedirected( arg );
	if ( num >= 0 )
	{
		//log( "read %08x\n", ( unsigned int )arg->arg );
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		int ret = hostfs_drv->funcs->IoRead( arg, data, len );
		arg->drv = drv;
		return ret;
	}
	return msIoRead( arg, data, len );
}

SceOff msIoLseek_new(PspIoDrvFileArg * arg, SceOff ofs, int whence )
{
	int num = isRedirected( arg );
	if ( num >= 0 )
	{
		//log( "seek %08x\n", ( unsigned int )arg->arg );
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		int ret = hostfs_drv->funcs->IoLseek( arg, ofs, whence );
		arg->drv = drv;
		return ret;
	}
	return msIoLseek( arg, ofs, whence );
}

int msIoIoctl_new( PspIoDrvFileArg * arg, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen )
{
	int num = isRedirected( arg );
	if ( num >= 0 )
	{
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		int ret = hostfs_drv->funcs->IoIoctl( arg, cmd, indata, inlen, outdata, outlen );
		arg->drv = drv;
		log( "ioctl %08x %08x %08x\n", ( unsigned int )arg->arg, cmd, ret );
		if ( ret < 0 )
			ret = 0;
		return ret;
	}
	return msIoIoctl( arg, cmd, indata, inlen, outdata, outlen );
}

int msIoDopen_new( PspIoDrvFileArg * arg, const char * dirname )
{
	int ret = msIoDopen( arg, dirname );
	if ( strstr( dirname , "/SAVEDATA" ) )
		return ret;
	if ( ret < 0 )
	{
		while( host_mode && !host_drv )
		{
			log( "waiting for host %s\n", dirname );
			sceKernelDelayThread( 1000000 );
		}
		log( "dopen %s\n", dirname );
		while ( init_key == PSP_INIT_KEYCONFIG_GAME && !host_drv )
			sceKernelDelayThread( 50000 );
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		ret = hostfs_drv->funcs->IoDopen( arg, dirname );
		arg->drv = drv;
		log( "try to dopen %s at host0 %08x\n", dirname, ( unsigned int )arg->arg );
		if ( ret >= 0 )
		{
			records[count ++] = arg->arg;
			if ( count % 32 == 0 )
			{
				void ** tmp = sceKernelAllocHeapMemory( heap_id, 4 * ( count + 32 ) );
				memcpy( tmp, records, 4 * count );
				sceKernelFreeHeapMemory( heap_id, records );
				records = tmp;
			}
		}
	}
	else
	{
		memcpy( &d_records[d_count].host_arg, arg, sizeof( PspIoDrvFileArg ) );
		d_records[d_count].host_arg.drv = host_drv;
		int res = hostfs_drv->funcs->IoDopen( &d_records[d_count].host_arg, dirname );
		log( "try to dopen  the 2nd %s at host0 %08x\n", dirname, ( unsigned int )d_records[d_count].host_arg.arg );
		if ( res >= 0 )
		{
			d_records[d_count].ms_arg = arg->arg;
			d_records[d_count ++].read_end = 0;
			if ( d_count % 32 == 0 )
			{
				DRecord * tmp = sceKernelAllocHeapMemory( heap_id, sizeof( DRecord ) * ( d_count + 32 ) );
				memcpy( tmp, d_records, sizeof( DRecord ) * d_count );
				sceKernelFreeHeapMemory( heap_id, d_records );
				d_records = tmp;
			}
		}
	}
	return ret;
}

int msIoDclose_new( PspIoDrvFileArg * arg )
{
	int num = isRedirected( arg );
	if ( num >= 0 )
	{
		log( "dclose %08x at host0\n", ( unsigned int )arg->arg );
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		int ret = hostfs_drv->funcs->IoDclose( arg );
		arg->drv = drv;
		count --;
		memcpy( &records[num], &records[num + 1], 4 * ( count - num ) );
		return ret;
	}
	num = isCombinated( arg );
	if ( num >= 0 )
	{
		log( "dclose the 2nd %08x at host0\n", ( unsigned int )d_records[num].host_arg.arg );
		hostfs_drv->funcs->IoDclose( arg );
		d_count --;
		memcpy( &d_records[num], &d_records[num + 1], sizeof( DRecord ) * ( d_count - num ) );
	}
	return msIoDclose( arg );
}

int msIoDread_new( PspIoDrvFileArg * arg, SceIoDirent * dir )
{
	int num = isRedirected( arg );
	if ( num >= 0 )
	{
		//log( "dread %08x\n", ( unsigned int )arg->arg );
		SceFatMsDirentPrivate * private = dir->d_private;
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		int ret = hostfs_drv->funcs->IoDread( arg, dir );
		arg->drv = drv;
		if ( ret >= 0 )
		{
			setDirentPrivate( private, dir );
		} 
		dir->d_private = private;
		return ret;
	}
	num = isCombinated( arg );
	if ( num >= 0 && !d_records[num].read_end )
	{
		//log( "dread 2nd %08x\n", ( unsigned int )d_records[num].host_arg.arg );
		SceFatMsDirentPrivate * private = dir->d_private;
		int ret = hostfs_drv->funcs->IoDread( &d_records[num].host_arg, dir );
		if ( ret >= 0 )
		{
			setDirentPrivate( private, dir );
		}
		if ( ret <= 0 )
			d_records[num].read_end = ret = 1;
		else
		{
			dir->d_private = private;
			return ret;
		}
	}
	return msIoDread( arg, dir );
}

int msIoGetstat_new( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat )
{
	int ret = msIoGetstat( arg, file, stat );
	if ( strstr( file , "/SAVEDATA" ) )
		return ret;
	if ( ret < 0 )
	{
		log( "getstat %s\n", file );
		while( host_mode && !host_drv )
		{
			log( "waiting for host %s\n", file );
			sceKernelDelayThread( 1000000 );
		}
		while ( init_key == PSP_INIT_KEYCONFIG_GAME && !host_drv )
			sceKernelDelayThread( 50000 );
		PspIoDrvArg * drv = arg->drv;
		arg->drv = host_drv;
		ret = hostfs_drv->funcs->IoGetstat( arg, file, stat );
		arg->drv = drv;
		log( "try to getstat %s at host0 %08x\n", file, ( unsigned int )arg->arg );
	}
	return ret;
}

int msIoChdir_new( PspIoDrvFileArg * arg, const char * dir )
{
	int ret = msIoChdir( arg, dir );
	PspIoDrvArg * drv = arg->drv;
	arg->drv = host_drv;
	ret = hostfs_drv->funcs->IoChdir( arg, dir );
	arg->drv = drv;
	return ret;
}

int onCtrlRead( SceCtrlData * pad_data, int count )
{
	if ( !pressed && ( pad_data[0].Buttons & hot_key ) == hot_key )
	{
		pressed = 1;
		sceKernelWakeupThread( thid );
	}
	if ( !ctrl_previous )
		return 0;
	return ctrl_previous( pad_data, count );
}

int restoreIoDrv()
{
	int intr = sceKernelCpuSuspendIntr();
	
	fatms_drv->funcs->IoOpen	= msIoOpen;
	fatms_drv->funcs->IoClose	= msIoClose;
	fatms_drv->funcs->IoRead	= msIoRead;
	//fatms_drv->funcs->IoWrite	= msIoWrite;
	fatms_drv->funcs->IoLseek	= msIoLseek;
	fatms_drv->funcs->IoIoctl	= msIoIoctl;
	//fatms_drv->funcs->IoRemove	= msIoRemove;
	//fatms_drv->funcs->IoMkdir	= msIoMkdir;
	//fatms_drv->funcs->IoRmdir	= msIoRmdir;
	fatms_drv->funcs->IoDopen	= msIoDopen;
	fatms_drv->funcs->IoDclose	= msIoDclose;
	fatms_drv->funcs->IoDread	= msIoDread;
	fatms_drv->funcs->IoGetstat	= msIoGetstat;
	//fatms_drv->funcs->IoChstat	= msIoChstat;
	//fatms_drv->funcs->IoRename	= msIoRename;
	fatms_drv->funcs->IoChdir	= msIoChdir;
	hostfs_drv->funcs->IoOpen	= hostfsIoOpen;
	
	sceKernelCpuResumeIntr( intr );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	
	sceKernelFreeHeapMemory( heap_id, d_records );
	sceKernelFreeHeapMemory( heap_id, records );
	sceKernelDeleteHeap( heap_id );
	return 0;
}

int patchMsDrv()
{
	fatms_drv = findDriver( "fatms" );
	if ( !fatms_drv )
	{
		log( "Error finding driver\n" );
		return -1;
	}
	msIoOpen		= fatms_drv->funcs->IoOpen;
	msIoClose		= fatms_drv->funcs->IoClose;
	msIoRead		= fatms_drv->funcs->IoRead;
	msIoLseek		= fatms_drv->funcs->IoLseek;
	msIoIoctl		= fatms_drv->funcs->IoIoctl;
	msIoDopen		= fatms_drv->funcs->IoDopen;
	msIoDclose		= fatms_drv->funcs->IoDclose;
	msIoDread		= fatms_drv->funcs->IoDread;
	msIoGetstat		= fatms_drv->funcs->IoGetstat;
	msIoChdir		= fatms_drv->funcs->IoChdir;
	
	int intr = sceKernelCpuSuspendIntr();
	
	fatms_drv->funcs->IoOpen	= msIoOpen_new;
	fatms_drv->funcs->IoClose	= msIoClose_new;
	fatms_drv->funcs->IoRead	= msIoRead_new;
	//fatms_drv->funcs->IoWrite	= msIoWrite_new;
	fatms_drv->funcs->IoLseek	= msIoLseek_new;
	fatms_drv->funcs->IoIoctl	= msIoIoctl_new;
	//fatms_drv->funcs->IoRemove	= msIoRemove_new;
	//fatms_drv->funcs->IoMkdir	= msIoMkdir_new;
	//fatms_drv->funcs->IoRmdir	= msIoRmdir_new;
	fatms_drv->funcs->IoDopen	= msIoDopen_new;
	fatms_drv->funcs->IoDclose	= msIoDclose_new;
	fatms_drv->funcs->IoDread	= msIoDread_new;
	fatms_drv->funcs->IoGetstat	= msIoGetstat_new;
	//fatms_drv->funcs->IoChstat	= msIoChstat_new;
	//fatms_drv->funcs->IoRename	= msIoRename_new;
	fatms_drv->funcs->IoChdir	= msIoChdir_new;
	
	sceKernelCpuResumeIntr( intr );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	return 0;
}

int patchHostDrv()
{
	hostfs_drv = findDriver( "host" );
	if ( !hostfs_drv )
	{
		log( "Error finding driver\n" );
		return -1;
	}
	
	heap_id = sceKernelCreateHeap( 1, 1024 * 10, PSP_SMEM_Low, "host2ms_heap" );
	if ( heap_id < 0 )
	{
		log( "Cant alloc heap!" );
		return -1;
	}
	records = sceKernelAllocHeapMemory( heap_id, 4 * 32 );
	d_records = sceKernelAllocHeapMemory( heap_id, sizeof( DRecord ) * 32 );
	count = 0;
	d_count = 0;
	
	hostfsIoOpen	= hostfs_drv->funcs->IoOpen;
	
	int intr = sceKernelCpuSuspendIntr();
	
	hostfs_drv->funcs->IoOpen	= hostfsIoOpen_new;
	
	sceKernelCpuResumeIntr( intr );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	
	sceIoOpen( "host:/poison.sup", PSP_O_RDONLY, 0644 );
	return 0;
}

int patchIoDrv()
{
	patchMsDrv();
	patchHostDrv();
	return 0;
}

int startHost( int device )
{
	int ret = -1;
	if ( device == D_USB )
	{
		ret = startUsbHost();
		if ( ret < 0 )
		{
			log( "Error starting USB Host\n" );
			return ret;
		}
		log( "started usbhost\n" );
	}
	else if ( device == D_WIFI )
	{
		ret = startWifiHost( NULL, &hc_conf );
		if ( ret < 0 )
		{
			log( "Error starting WIFI Host\n" );
			return ret;
		}
	}
	return 0;
}

int stopHost( int device )
{
	int ret = -1;
	if ( device == D_USB )
	{
		ret = stopUsbHost();
		if ( ret < 0 )
		{
			log( "Error stopping USB Host\n" );
			return ret;
		}
		log( "stopped usbhost\n" );
	}
	else if ( device == D_WIFI )
	{
		ret = stopWifiHost();
		if ( ret < 0 )
		{
			log( "Error stopping WIFI Host\n" );
			return ret;
		}
	}
	return 0;
}

int exitCallback( int arg1, int arg2, void * common )
{
	log( "exit game\n" );
	initUmdImageDriver();
	restoreIoDrv();
	stopHost( host_mode );
	exitVshWithError( 0 );
	return 0;
}

int callbackThread( SceSize args, void * argp )
{
	int cbid;
	cbid = sceKernelCreateCallback( "Exit Callback", exitCallback, NULL );
	sceKernelRegisterExitCallback( cbid );
	sceKernelSleepThreadCB();
	return 0;
}

int setupCallbacks()
{
	int tid = sceKernelCreateThread( "update_thread", callbackThread, 0x11, 0x800, 0, 0 );
	if( tid >= 0 )
	{
		sceKernelStartThread(tid, 0, 0);
	}
	return tid;
}

void redirect(int mode)
{
	int ori = host_mode;

	host_mode = mode;

	if ( ori != host_mode )
	{
		if ( ori )
		{
			restoreIoDrv();
			stopHost( ori );
		}
		if ( host_mode )
		{
			int ret = startHost( host_mode );
			if ( ret < 0 )
			{
				host_mode = 0;
			}
			else
			{
				patchIoDrv();
			}
		}
		else sceIoRemove( MODE_FILE );
	}
}

int main_thread( SceSize args, void *argp )
{
	sceIoRemove("ms0:/seplugins/cfe/log.txt");

	if ( init_key == PSP_INIT_KEYCONFIG_GAME && host_mode )
	{
		startHost( host_mode );
		patchHostDrv();
		killModule( "cfe_launcher" );
		int ret = -1;
		if ( strcmp( &exec[strlen( exec ) - 9], "EBOOT.PBP" ) == 0 )
		{
			setInitFileName( exec );
			ret = sceKernelLoadModuleForLoadExecVSHMs2( exec, 0, NULL );
			log( "loadexec ret %08x\n", ret );
			if ( ret >= 0 )
			{
				int status, len = strlen( exec ) + 1;
				ret = sceKernelStartModule( ret, len, ( void * )exec, &status, NULL );
				log( "start ret %08x\n", ret );
			}
		}
		else
		{
			initUmdImageDriver();
			mountUmdImage( exec );
			ret = launchUmdImage();
		}
		if ( ret < 0 )
		{
			initUmdImageDriver();
			restoreIoDrv();
			stopHost( host_mode );
			exitVshWithError( ret );
			return 0;
		}
		int cbid;
		while( ( cbid = sceKernelCheckExitCallback() ) == 0 )
		{
			sceKernelDelayThread( 1000000 );
		}
		sceKernelDelayThread( 2000000 );
		cbid = sceKernelCheckExitCallback();
		sceKernelUnregisterExitCallback();
		sceKernelDeleteCallback( cbid );
		setupCallbacks();
	}
	if ( init_key != PSP_INIT_KEYCONFIG_VSH )
		goto TermExit;

	while(1)
	{
		sceKernelDelayThread( 2000000 );
	}
	
/*
	hot_key = strToHex( hc_conf.key );
	initCtrl();
	RawImageBlitParams blit_opts;
	memset( &blit_opts, 0, sizeof( RawImageBlitParams ) );
	blit_opts.width = 89;
	blit_opts.height = 30;
	blit_opts.x = 195;
	blit_opts.y = 121;
	initTinyUi();
	ctrl_previous = setCtrlReadHandler( onCtrlRead );
	while ( 1 )
	{
		sceKernelSleepThread();
		int bid = sceKernelAllocPartitionMemory( 2, "tinyui_blit", PSP_SMEM_Low, blit_opts.width * blit_opts.height * 4, NULL );
		if ( bid < 0 )
		{
			log( "Error allocating mem\n" );
			continue;
		}
		ctrlLock();
		blit_opts.sema = 1;
		blit_opts.data = sceKernelGetBlockHeadAddr( bid );
		readFile( icons[host_mode], blit_opts.data, blit_opts.width * blit_opts.height * 4 );
		blitRawImage( &blit_opts );
		unsigned int pressed_key = 1;
		int ori = host_mode;
		while( ( pressed_key = ctrlWaitKey( PSP_CTRL_LEFT | PSP_CTRL_RIGHT, 3 ) ) )
		{
			if ( pressed_key == PSP_CTRL_LEFT )
				host_mode = host_mode == 0? 2: ( host_mode - 1 );
			else if ( pressed_key == PSP_CTRL_RIGHT )
				host_mode = host_mode == 2? 0: ( host_mode + 1 );
			else break;
			readFile( icons[host_mode], blit_opts.data, blit_opts.width * blit_opts.height * 4 );
		}
		if ( ori != host_mode )
		{
			if ( ori )
			{
				restoreIoDrv();
				stopHost( ori );
			}
			if ( host_mode )
			{
				int ret = startHost( host_mode );
				if ( ret < 0 )
				{
					host_mode = 0;
					readFile( icons[0], blit_opts.data, blit_opts.width * blit_opts.height * 4 );
				}
				else
				{
					patchIoDrv();
				}
			}
			else sceIoRemove( MODE_FILE );
		}
		blit_opts.sema = 0;
		pressed = 0;
		ctrlUnlock();
		sceKernelFreePartitionMemory( bid );
	}
*/
TermExit:
	return sceKernelExitDeleteThread( 0 );
}

int ( * LoadExecVSHCommon )( int apitype, char * file, struct SceKernelLoadExecVSHParam * param, int unk2 );
int LoadExecVSHCommon_new( int apitype, char * file, struct SceKernelLoadExecVSHParam * param, int unk2 )
{
	log("Entering LoadExecVSHCommon_new\n");
	restoreLoadExecVSHCommon();
	if ( host_mode )
	{
		restoreIoDrv();
		stopHost( host_mode );
		char args[256];
		char * argv[3];
		argv[0] = LAUNCHER;
		argv[1] = ( char * )mode_str[host_mode];
		argv[2] = NULL;
		if ( strncmp( file, "disc0", 5 ) == 0 )
		{
			char * umd_file = getUmdFile();
			int fd = sceIoOpen( umd_file, PSP_O_RDONLY, 0644 );
			if ( fd < 0 )
			{
				argv[2] = umd_file;
				param->args = buildArgs( args, 3, argv );
				setUmdFile( "ms0:/seplugins/cfe/fake.iso" );
			}
			sceIoClose( fd );
		}
		else
		{
			int fd = sceIoOpen( file, PSP_O_RDONLY, 0644 );
			if ( fd < 0 )
			{
				argv[2] = file;
				param->args = buildArgs( args, 3, argv );
			}
			sceIoClose( fd );
		}
		if ( argv[2] )
		{
			param->argp = args;
			return LoadExecVSHCommon( 0x141, LAUNCHER, param, 0x00010000 );
		}
	}
	return LoadExecVSHCommon( apitype, file, param, unk2 );
}

int module_start( SceSize args, void *argp )
{
	if ( initPatches() < FW_371 )
	{
		log( "Not supported firmware version!\n" );
		return 0;
	}
	sceIoAssign("ms0:", "msstor0p1:", "fatms0:", IOASSIGN_RDWR, NULL, 0);
	initUtils();
	init_key = sceKernelInitKeyConfig();
	log( "init_key %08x\n", init_key );
	readConf( &hc_conf );
	if ( init_key == PSP_INIT_KEYCONFIG_GAME )
	{
		patchMemPartitionInfo();
		char * h_mode = ( char * )0x09000000;
		char * f_exec = ( char * )0x09000010;
		log( "mode %s, exec %s\n", h_mode, f_exec );
		strcpy( exec, f_exec );
		if ( !strncmp( h_mode, mode_str[1], 3 ) )
		{
			host_mode = D_USB;
		}
		else if ( !strncmp( h_mode, mode_str[2], 3 ) )
		{
			host_mode = D_WIFI;
		}
		if ( host_mode )
		{
			patchMsDrv();
		}
		else return 0;
	}
	else if ( init_key == PSP_INIT_KEYCONFIG_VSH )
		LoadExecVSHCommon = patchLoadExecVSHCommon( LoadExecVSHCommon_new );
	thid = sceKernelCreateThread( "host2ms_thread", main_thread, 0x18, 0x4000, 0x00100001, NULL );
	if( thid >= 0 )
		sceKernelStartThread( thid, 0, 0 );
	return 0;
}

int module_stop( SceSize args, void *argp )
{
	return 0;
}
