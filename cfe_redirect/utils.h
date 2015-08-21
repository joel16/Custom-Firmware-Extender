/*
 *	utils.h is part of host2ms
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
 *	Date Created:	2008-04-04
 */

#pragma once

#include <pspkernel.h>
#include <psploadexec_kernel.h>

#define J_OPCODE	0x08000000
#define JAL_OPCODE	0x0C000000
#define NOP	0x00000000
#define MAKE_JUMP( a, f ) _sw( J_OPCODE | ( ( ( unsigned int )( f ) & 0x0ffffffc ) >> 2 ), a )
#define MAKE_CALL( a, f ) _sw( JAL_OPCODE | ( ( ( unsigned int )( f ) >> 2 )  & 0x03ffffff ), a )

enum PspModel
{
	PSP_MODEL_STANDARD = 0,
	PSP_MODEL_SLIM_AND_LITE = 1
};

enum PspFwVersion
{
	FW_371 = 0x03070110,
	FW_380 = 0x03080010,
	FW_390 = 0x03090010,
	FW_401 = 0x04000110,
	FW_500 = 0x05000010,
	FW_620 = 0x06020010,
	FW_635 = 0x06030510,
	FW_639 = 0x06030910,
	FW_660 = 0x06060010,
	FW_661 = 0x06060110,
};

typedef struct tSceModule
{
	struct tSceModule	*next; //0x00
	unsigned short		attribute; //0x04
	unsigned char		version[2]; //0x06
	char				modname[27]; //0x08
	char				terminal; //0x23
	unsigned int		unknown1; //0x24
	unsigned int		unknown2; //0x28
	int					modid; //0x2c
	unsigned int		unknown3[4]; //0x30
	void *				ent_top;  //0x40
	unsigned int		ent_size; //0x44
	void *				stub_top; //0x48
	unsigned int		stub_size; //0x4c
	unsigned int		unknown4[5]; //0x50
	unsigned int		entry_addr; //0x64
	unsigned int		gp_value; //0x68
	unsigned int		text_addr; //0x6c
	unsigned int		text_size;
	unsigned int		data_size;
	unsigned int		bss_size;
	unsigned int		nsegment;
	unsigned int		segmentaddr[4];
	unsigned int		segmentsize[4];
} tSceModule;

typedef struct CfwConfig
{
	int magic; /* 0x47434553 */ //0
	int hidecorrupt; //4
	int	skiplogo; //8
	int umdactivatedplaincheck; //c
	int gamekernel150; // 10
	int executebootbin; //14
	int startupprog; //18
	int umdmode; //1c
	int useisofsonumdinserted; //20
	int	vshcpuspeed;  //24
	int	vshbusspeed;  //28
	int	umdisocpuspeed; //2c
	int	umdisobusspeed; //30
	int fakeregion; //34
	int freeumdregion; //38
	int	hardresetHB; //3c
	int usbdevice; //40
	int novshmenu; //44
	int dummy[10];
} CfwConfig;

typedef int (* StartModuleHandler)(tSceModule *);

extern int readFile( const char * file, void * buf, int size );

extern int writeFile( const char * file, void * buf, int size );

extern unsigned int strToHex( const char * str );

extern int stripSpace( char * str );

extern int readLine( int fd, char * buf, int max_len );

extern PspIoDrv * findDriver( char * drvname );

extern int patchModule( const char * szMod, char mask );

extern unsigned int findProc( const char * szMod, const char * szLib, unsigned int nid );

extern void * patchExport( const char * szMod, const char * szLib, unsigned int nid, void * new_func );

extern int findUIDByName( const char * name, const char * parent );

extern int killThread( const char * parent, const char * name );

extern int killSema( const char * parent, const char * name );

extern int killEventFlag( const char * parent, const char * name );

extern int killMutex( const char * parent, const char * name );

extern int killEventHandler( const char * name );

extern int killModule( const char * name );

extern int buildArgs( char * args, int argc, char ** argv );

extern int parseArgs( char ** argv, int args, char * argp );

extern int loadStartModule( char * file, int argc, char ** argv );

extern int loadStartModulePartition( int pid, char * file, int argc, char ** argv );

extern int loadStartUserModule( char * file, int argc, char ** argv );

extern int getRegistryValue( const char * dir, const char * name, unsigned int * val );

extern void exitVshWithError( unsigned int err );

extern void initUtils();

extern void ( * setUmdFile )( const char * file );

extern char * ( * getUmdFile )();

extern int ( * getCfwConfig )( CfwConfig * config );

extern int ( * setInitApitype )( int apitype );

extern int ( * setInitFileName )( char * file );

extern int ( * mountUmdFromFile )( char * file, int noumd, int isofs );

extern void ( * unmountUmd )( void );

extern int ( * sceKernelUnregisterExitCallback )( void );

extern int ( * sceKernelCheckExitCallback )( void );

extern int ( * sceKernelLoadModuleForLoadExecVSHMs2 )( const char * file, int flags, SceKernelLMOption * option );

extern int ( * sceKernelLoadModuleForLoadExecVSHDisc )( const char * file, int flags, SceKernelLMOption * option );

extern void initNetHostFuncs();
