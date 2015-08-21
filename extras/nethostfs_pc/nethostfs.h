#ifndef __NETHOSTFS_H__

#define __NETHOSTFS_H__

#ifdef __psp__
#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>
#include <pspiofilemgr_dirent.h>
#include <pspiofilemgr_stat.h>
#include <pspiofilemgr_kernel.h>
#else
#include "pspiofilemgr.h"
#endif

#define PACKEDSTRUCT struct __attribute__((packed))	

#define CHALLENGE_TEXT_LEN	8

enum
{
	NET_HOSTFS_CMD_HELLO = 0x00DA5800,
	NET_HOSTFS_CMD_IOINIT,
	NET_HOSTFS_CMD_IOEXIT,
	NET_HOSTFS_CMD_IOOPEN,
	NET_HOSTFS_CMD_IOCLOSE,
	NET_HOSTFS_CMD_IOREAD,
	NET_HOSTFS_CMD_IOWRITE,
	NET_HOSTFS_CMD_IOLSEEK,
	NET_HOSTFS_CMD_IOIOCTL,
	NET_HOSTFS_CMD_IOREMOVE,
	NET_HOSTFS_CMD_IOMKDIR,
	NET_HOSTFS_CMD_IORMDIR,
	NET_HOSTFS_CMD_IODOPEN,
	NET_HOSTFS_CMD_IODCLOSE,
	NET_HOSTFS_CMD_IODREAD,
	NET_HOSTFS_CMD_IOGETSTAT,
	NET_HOSTFS_CMD_IOCHSTAT,
	NET_HOSTFS_CMD_IORENAME,
	NET_HOSTFS_CMD_IODEVCTL,
	NET_HOSTFS_CMD_GETKEY		//GETKEY
} NetHostFSCommand;

typedef PACKEDSTRUCT IO_GENERIC_FILE_PARAMS
{
	char	file[256];
	u32		fs_num; // Ignored at the moment
} IO_GENERIC_FILE_PARAMS;

typedef PACKEDSTRUCT IO_GENERIC_DIR_PARAMS
{
	char	dir[256];
	u32		fs_num; // Ignored at the moment
} IO_GENERIC_DIR_PARAMS;

typedef PACKEDSTRUCT IO_OPEN_PARAMS
{
	char	file[256];
	u32		fs_num; // Ignored at the moment
	int		flags;
	SceMode	mode;
} IO_OPEN_PARAMS;

typedef PACKEDSTRUCT IO_LOGIN_CHALLENGE_PARAMS
{
	unsigned char	challenge[CHALLENGE_TEXT_LEN];
} IO_LOGIN_CHALLENGE_PARAMS;

typedef PACKEDSTRUCT IO_LOGIN_RESPONSE_PARAMS
{
	unsigned char	response[CHALLENGE_TEXT_LEN];
} IO_LOGIN_RESPONSE_PARAMS;

typedef PACKEDSTRUCT IO_GETKEY_INPUTPARAMS
{
	int p2type;
} IO_GETKEY_INPUTPARAMS;

typedef PACKEDSTRUCT IO_GETKEY_PARAMS
{
	int res;
	int but2;
	unsigned char xaxis;
	unsigned char yaxis;
	unsigned char xaxis2;
	unsigned char yaxis2;
} IO_GETKEY_PARAMS;

typedef PACKEDSTRUCT IO_READ_PARAMS
{
	int fd;
	int len;
} IO_READ_PARAMS;

typedef IO_READ_PARAMS IO_WRITE_PARAMS;

typedef PACKEDSTRUCT IO_LSEEK_PARAMS
{
	int			fd;
	SceOff		offset;
	int			whence;
} IO_LSEEK_PARAMS;

typedef IO_GENERIC_FILE_PARAMS IO_REMOVE_PARAMS;

typedef PACKEDSTRUCT IO_MKDIR_PARAMS
{
	char	dir[256];
	u32		fs_num; // Ignored at the moment
	int		mode;
} IO_MKDIR_PARAMS;

typedef IO_GENERIC_DIR_PARAMS IO_RMDIR_PARAMS;

typedef IO_GENERIC_DIR_PARAMS IO_DOPEN_PARAMS;

typedef PACKEDSTRUCT IO_DREAD_RESULT
{
	int	res;
	SceIoDirent	entry;
} IO_DREAD_RESULT;

typedef IO_GENERIC_FILE_PARAMS IO_GETSTAT_PARAMS;

typedef PACKEDSTRUCT IO_GETSTAT_RESULT
{
	int res;
	SceIoStat stat;
} IO_GETSTAT_RESULT;

typedef PACKEDSTRUCT IO_CHSTAT_PARAMS
{
	char file[256];
	int	 fs_num;
	SceIoStat stat;
	int bits;
} IO_CHSTAT_PARAMS;

typedef PACKEDSTRUCT IO_RENAME_PARAMS
{
	char oldfile[256];
	char newfile[256];
	int	 fs_num;
} IO_RENAME_PARAMS;

#endif /* __NETHOSTFS_H__ */
