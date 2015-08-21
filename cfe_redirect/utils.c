/*
 *	utils.c is part of host2ms
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
#include <pspsdk.h>
#include <pspreg.h>
#include <pspsysmem_kernel.h>
#include <psploadcore.h>
#include <pspsysevent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "mem.h"
#include "syspatch.h"
#include "log.h"

#define HEX_VALUE( ch ) ch >= 97? ( ch - 97 ): ( ch >= 65? ( ch - 64 ): ( ch - 48 ) )

int readFile( const char * file, void * buf, int size )
{
	int fd = sceIoOpen( file, PSP_O_RDONLY, 0644 );
	if ( fd < 0 )
		return fd;
	sceIoRead( fd, buf, size );
	sceIoClose( fd );
	return 0;
}

int writeFile( const char * file, void * buf, int size )
{
	int fd = sceIoOpen( file, PSP_O_CREAT | PSP_O_RDWR | PSP_O_TRUNC, 0777 );
	if ( fd < 0 )
		return fd;
	sceIoWrite( fd, buf, size );
	sceIoClose( fd );
	return 0;
}

unsigned int strToHex( const char * str )
{
	int i = 0, len = strlen( str );
	unsigned int res = 0;
	if ( len > 8 )
		return 0;
	while ( i < len )
	{
		res += HEX_VALUE( str[len - i  - 1] ) << ( i * 4 );
		i ++;
	}
	return res;
}

static int isSpace( int ch )
{
	if( ( ch == ' ' ) || ( ch == '\t' ) || ( ch == '\n' ) || ( ch == '\r' ) )
		return 1;
	return 0;
}

int stripSpace( char * str )
{
	int start =  0, end = strlen( str ) - 1, pos;
	while ( end >= 0 )
	{
		if ( isSpace( str[end] ) )
		{
			str[end] = 0;
			end --;
		}
		else break;
	}
	while ( str[start] )
	{
		if ( isSpace( str[start] ) )
			start ++;
		else break;
	}
	pos = end + 1;
	if ( start > 0 )
	{
		pos = 0;
		while ( str[start] )
			str[pos ++] = str[start ++];
		str[pos] = 0;
	}
	return pos;
}

int readLine( int fd, char * buf, int max_len )
{
	int i = 0, bytes;
	while( i < max_len && ( bytes = sceIoRead( fd, buf + i, 1 ) ) == 1 )
	{
		if ( buf[i] == -1 || buf[i] == '\n' )
			break;
		i ++;
	}
	buf[i] = 0;
	if ( bytes != 1 && i == 0 )
		return -1;
	return i;
}

PspIoDrv * findDriver( char * drvname )
{
	unsigned int * ( * getDevice )( char * ) = ( void * )getFindDriverAddr();
	if ( !getDevice )
		return NULL;
	unsigned int * u;
	u = getDevice( drvname );
	if ( !u )
		return NULL;
	log( "%s found!\nu0: %08x\nu1: %08x\nu2: %08x\nu3: %08x\n",
		drvname, u[0], u[1], u[2], u[3] );
	return ( PspIoDrv * )u[1];
}

int patchModule( const char * szMod, char mask )
{
	SceLibraryEntryTable *entry;
	tSceModule *pMod;
	void *entTab;
	int entLen;
	pMod = ( tSceModule * )sceKernelFindModuleByName( szMod );
	if ( !pMod )
	{
		return 0;
	}
	if ( ( pMod->stub_top - pMod->ent_top ) < 40 )
	{
		return 1;
	}
	pMod->attribute = 0x1006;
	pMod->modname[0] = mask;
	int i = 0;
	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	while( i < entLen )
	{
		entry = ( SceLibraryEntryTable * )( entTab + i );

		if( entry->libname )
		{
			( ( char * )entry->libname )[0] = mask;
		}

		i += ( entry->len * 4 );
	}
	return 1;
}

unsigned int * findExport( const char * szMod, const char * szLib, unsigned int nid )
{
	SceLibraryEntryTable *entry;
	tSceModule *pMod;
	void *entTab;
	int entLen;
	pMod = ( tSceModule * )sceKernelFindModuleByName( szMod );
	if ( !pMod )
	{
		return 0;
	}
	int i = 0;
	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	while( i < entLen )
	{
		int count;
		int total;
		unsigned int *vars;

		entry = ( SceLibraryEntryTable * )( entTab + i );

		if( entry->libname && ( !szLib || !strcmp( entry->libname, szLib ) ) )
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			for( count = 0; count < entry->stubcount; count ++ )
			{
				if ( vars[count] == nid )
				{
					return &vars[count+total];	
				}				
			}
		}

		i += ( entry->len * 4 );
	}
	return NULL;
}

unsigned int findProc( const char * szMod, const char * szLib, unsigned int nid )
{
	unsigned int * export = findExport( szMod, szLib, nid );
	if( export )
	{
		log( "func %08x in %s of %s found:\n%08x\n",
			nid, szLib, szMod, *export );
		return *export;
	}
	return 0;
}

void * patchExport( const char * szMod, const char * szLib, unsigned int nid, void * new_func )
{
	unsigned int * export = findExport( szMod, szLib, nid );
	void * ori = NULL;
	if ( export )
	{
		ori = ( void * )*export;
		*export = ( unsigned int )new_func;
		log( "func %08x in %s of %s patched:\n%08x -> %08x\n",
			nid, szLib, szMod, ( unsigned int )ori, *export );
		return ori;
	}
	return 0;
}

int findUIDByName( const char * name, const char * parent )
{
	uidControlBlock * entry;
	uidControlBlock * end;

	entry = SysMemForKernel_536AD5E1();
	
	entry = entry->parent;
	end = entry;
	entry = entry->nextEntry;

	do {
		if ( entry->nextChild != entry )
		{
			do {
				uidControlBlock * ret = NULL;
				entry = entry->nextChild;
				if( name )
				{
					if ( strcmp( entry->name, name ) == 0 )
						ret = entry;
				}

				if( ret )
				{
					if( parent && ret->type )
					{
						if( strcmp( parent, ret->type->name ) == 0 )
						{
							return ret->UID;
						}
					}
					else
					{
						return ret->UID;
					}
				}

			} while ( entry->nextChild != entry->type );
			entry = entry->nextChild;
		}
		entry = entry->nextEntry;
	} while ( entry->nextEntry != end );
	return 0;
}

int killThread( const char * parent, const char * name )
{
	int id = findUIDByName( name, parent );
	if ( !id )
		return -1;
	return sceKernelTerminateDeleteThread( id );
}

int killSema( const char * parent, const char * name )
{
	int id = findUIDByName( name, parent );
	if ( !id )
		return -1;
	return sceKernelDeleteSema( id );
}

int killEventFlag( const char * parent, const char * name )
{
	int id = findUIDByName( name, parent );
	if ( !id )
		return -1;
	return sceKernelDeleteEventFlag( id );
}

int killMutex( const char * parent, const char * name )
{
	int id = findUIDByName( name, parent );
	if ( !id )
		return -1;
	int ( * sceKernelDeleteMutex )( int );
	sceKernelDeleteMutex = ( void * )findProc( "sceThreadManager", "ThreadManForKernel", getKillMutexNid() );
	return sceKernelDeleteMutex( id );
}

int killEventHandler( const char * name )
{
	PspSysEventHandler * seh = sceKernelReferSysEventHandler();
	while( seh )
	{
		if ( strcmp( seh->name, name ) )
			return sceKernelUnregisterSysEventHandler( seh );
		seh = seh->next;
	}
	return -1;
}

int killModule( const char * name )
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( name );
	if ( !pMod )
		return -1;
	if ( ( ( unsigned int )pMod & 0xff000000 ) == 0x88000000 )
		pMod->attribute = 0x1006;
	int ret = sceKernelStopModule( pMod->modid, 0, NULL, NULL, NULL );
	sceKernelDelayThread( 100000 );
	log( "stop module %s: %08x ", name, ret );
	ret = sceKernelUnloadModule( pMod->modid );
	log( "unload: %08x\n", ret );
	return ret;
}

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

int loadStartModule( char * file, int argc, char ** argv )
{
	int modid = sceKernelLoadModule( file, 0, NULL );
	log( "load module %s %08x\n", file, modid );
	if ( modid >= 0 )
	{
		char args[512];
		int status, len = 0;
		strcpy( args, file );
		len += strlen( file ) + 1;
		len += buildArgs( &args[len], argc, argv );
		int ret = sceKernelStartModule( modid, len, ( void * )args, &status, NULL );
		log( "start module %08x ret %08x\n", modid, ret );
		if ( ret < 0 )
		{
			int res = -1;
			res = sceKernelUnloadModule( modid );
			log( "unload: %08x\n", res );
			return ret;
		}
	}
	return modid;
}

int loadStartModulePartition( int pid, char * file, int argc, char ** argv )
{
	SceKernelLMOption option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = pid;
	option.mpiddata = pid;
	option.position = pid == 4? PSP_SMEM_High: PSP_SMEM_Low;
	option.access = 1;
	int modid = sceKernelLoadModule( file, 0, &option );
	log( "load module %s %08x\n", file, modid );
	if ( modid >= 0 )
	{
		char args[512];
		int status, len = 0;
		strcpy( args, file );
		len += strlen( file ) + 1;
		len += buildArgs( &args[len], argc, argv );
		int ret = sceKernelStartModule( modid, len, ( void * )args, &status, NULL );
		log( "start module %08x ret %08x\n", modid, ret );
		if ( ret < 0 )
		{
			int res = -1;
			res = sceKernelUnloadModule( modid );
			log( "unload: %08x\n", res );
			return ret;
		}
	}
	return modid;
}

int loadStartUserModule( char * file, int argc, char ** argv )
{
	return loadStartModulePartition( PSP_MEMORY_PARTITION_USER, file, argc, argv );
}

int getRegistryValue( const char * dir, const char * name, unsigned int * val )
{
	int ret = 0;
    struct RegParam reg;
    REGHANDLE h;

    memset( &reg, 0, sizeof( reg ) );
    reg.regtype = 1;
    reg.namelen = strlen( "/system" );
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy( reg.name, "/system" );
    if( sceRegOpenRegistry( &reg, 2, &h ) == 0 )
    {
            REGHANDLE hd;
            if( !sceRegOpenCategory( h, dir, 2, &hd ) )
            {
                    REGHANDLE hk;
                    unsigned int type, size;

                    if( !sceRegGetKeyInfo( hd, name, &hk, &type, &size ) )
                    {
                            if( !sceRegGetKeyValue( hd, hk, val, 4 ) )
                            {
                                    ret = 1;
                                    sceRegFlushCategory( hd );
                            }
                    }
                    sceRegCloseCategory( hd );
            }
            sceRegFlushRegistry( h );
            sceRegCloseRegistry( h );
    }
    return ret;
}

void ( * setUmdFile )( const char * file );
char * ( * getUmdFile )( void );
int ( * getCfwConfig )( CfwConfig * config );
int ( * setInitApitype )( int apitype );
int ( * setInitFileName )( char * file );
int ( * mountUmdFromFile )( char * file, int noumd, int isofs );
void ( * unmountUmd )( void );
int ( * sceKernelExitVSH )( struct SceKernelLoadExecVSHParam *param );
int ( * sceKernelUnregisterExitCallback )( void );
int ( * sceKernelCheckExitCallback )( void );
int ( * sceKernelLoadModuleForLoadExecVSHMs2 )( const char * file, int flags, SceKernelLMOption * option );
int ( * sceKernelLoadModuleForLoadExecVSHDisc )( const char * file, int flags, SceKernelLMOption * option );

void exitVshWithError( unsigned int err )
{
	if ( err == 0 )
		sceKernelExitVSH( NULL );
	struct SceKernelLoadExecVSHParam param;
	unsigned int vsh_args[8];
	memset( &param, 0, sizeof( struct SceKernelLoadExecVSHParam ) );
	memset( vsh_args, 0, 0x20 );
	
	vsh_args[0] = 0x400;
	vsh_args[1] = 0x20;
	vsh_args[5] = err;
	
	param.size = sizeof( struct SceKernelLoadExecVSHParam );
	param.args = 0x400;
	param.argp = vsh_args;
	param.vshmain_args_size = 0x400;
	param.vshmain_args = vsh_args;
	sceKernelExitVSH( &param );
}

/*typedef struct PBPHeader
{
   char signature[4];
   int version;
   int offset[8];
} PBPHeader;

int loadStartPBP( const char * file )
{
	PBPHeader pbp;
	memset( &pbp, 0, sizeof( PBPHeader ) );
	int fd = sceIoOpen( file, PSP_O_RDONLY, 0644 );
	if ( fd < 0 )
		return -3;
	sceIoRead( fd, &pbp, sizeof( PBPHeader ) );
	int size = pbp.offset[7] - pbp.offset[6];
	int pbp_size = 64 - size % 64 + size;
	void * pbp_buf = memAlloc( pbp_size, 2 );
	memset( pbp_buf, 0, pbp_size );
	sceIoLseek( fd, pbp.offset[6], PSP_SEEK_SET );
	sceIoRead( fd, pbp_buf, size );
	sceIoClose( fd );
	int ( * sceKernelLoadModuleBuffer )( SceSize bufsize, void *buf, int flags, SceKernelLMOption *option );
	sceKernelLoadModuleBuffer = ( void * )findProc( "sceModuleManager", "ModuleMgrForKernel", 0x96817B71 );
	int ret = sceKernelLoadModuleBuffer( pbp_size, pbp_buf, 0, NULL );
	//int ( * sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlan )( void *buf, SceSize bufsize, int flags, SceKernelLMOption *option );
	//sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlan = ( void * )findProc( "sceModuleManager", "ModuleMgrForKernel", 0x1F0F8DF2 );
	//int ret = sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlan( pbp_buf, pbp_size, 0, NULL );
	log( "loadexec ret %08x\n", ret );
	if ( ret >= 0 )
	{
		int status, len = strlen( file ) + 1;
		ret = sceKernelStartModule( ret, len, ( void * )file, &status, NULL );
		log( "start ret %08x\n", ret );
	}
	return ret;
}*/

void initUtils()
{
	setUmdFile = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0xB64186D0 );
	getUmdFile = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0xAC56B90B );
	setInitApitype = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0x8d5be1f0 );
	setInitFileName = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0x128112c3 );
	mountUmdFromFile = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0x85b520c6 );
	unmountUmd = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0x512e0cd8 );
	getCfwConfig = ( void * )findProc( "SystemControl", "SystemCtrlForKernel", 0x16c3b7ee );
	
	unsigned int nid[5];
	getUtilsNids( nid );
	sceKernelExitVSH = ( void * )findProc( "sceLoadExec", "LoadExecForKernel", nid[0] );
	sceKernelUnregisterExitCallback = ( void * )findProc( "sceLoadExec", "LoadExecForKernel", nid[1] );
	sceKernelCheckExitCallback = ( void * )findProc( "sceLoadExec", "LoadExecForKernel", nid[2] );
	sceKernelLoadModuleForLoadExecVSHMs2 = ( void * )findProc( "sceModuleManager", "ModuleMgrForKernel", nid[3] );
	sceKernelLoadModuleForLoadExecVSHDisc = ( void * )findProc( "sceModuleManager", "ModuleMgrForKernel", nid[4] );
}
