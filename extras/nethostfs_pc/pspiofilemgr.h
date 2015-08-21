#ifndef __PSPIOFILEMGR_H__

#define __PSPIOFILEMGR_H__

#include <stdint.h>

#ifndef u8
#define u8	unsigned char
#endif 

#ifndef u16
#define	u16	unsigned short
#endif

#ifndef u32
#define u32	unsigned long
#endif

#define PSP_O_RDONLY	0x0001
#define PSP_O_WRONLY	0x0002
#define PSP_O_RDWR		(PSP_O_RDONLY | PSP_O_WRONLY)
#define PSP_O_NBLOCK	0x0004
//#define PSP_O_DIROPEN	0x0008	
#define PSP_O_APPEND	0x0100
#define PSP_O_CREAT		0x0200
#define PSP_O_TRUNC		0x0400
#define	PSP_O_EXCL		0x0800
#define PSP_O_NOWAIT	0x8000

#define PSP_SEEK_SET	0
#define PSP_SEEK_CUR	1
#define PSP_SEEK_END	2

/** Access modes for st_mode in SceIoStat (confirm?). */
enum IOAccessModes
{
	/** Format bits mask */
	FIO_S_IFMT		= 0xF000,
	/** Symbolic link */
	FIO_S_IFLNK		= 0x4000,
	/** Directory */
	FIO_S_IFDIR		= 0x1000,
	/** Regular file */
	FIO_S_IFREG		= 0x2000,

	/** Set UID */
	FIO_S_ISUID		= 0x0800,
	/** Set GID */
	FIO_S_ISGID		= 0x0400,
	/** Sticky */
	FIO_S_ISVTX		= 0x0200,

	/** User access rights mask */
	FIO_S_IRWXU		= 0x01C0,	
	/** Read user permission */
	FIO_S_IRUSR		= 0x0100,
	/** Write user permission */
	FIO_S_IWUSR		= 0x0080,
	/** Execute user permission */
	FIO_S_IXUSR		= 0x0040,	

	/** Group access rights mask */
	FIO_S_IRWXG		= 0x0038,	
	/** Group read permission */
	FIO_S_IRGRP		= 0x0020,
	/** Group write permission */
	FIO_S_IWGRP		= 0x0010,
	/** Group execute permission */
	FIO_S_IXGRP		= 0x0008,

	/** Others access rights mask */
	FIO_S_IRWXO		= 0x0007,	
	/** Others read permission */
	FIO_S_IROTH		= 0x0004,	
	/** Others write permission */
	FIO_S_IWOTH		= 0x0002,	
	/** Others execute permission */
	FIO_S_IXOTH		= 0x0001,	
};

// File mode checking macros
#define FIO_S_ISLNK(m)	(((m) & FIO_S_IFMT) == FIO_S_IFLNK)
#define FIO_S_ISREG(m)	(((m) & FIO_S_IFMT) == FIO_S_IFREG)
#define FIO_S_ISDIR(m)	(((m) & FIO_S_IFMT) == FIO_S_IFDIR)

/** File modes, used for the st_attr parameter in SceIoStat (confirm?). */
enum IOFileModes
{
	/** Format mask */
	FIO_SO_IFMT			= 0x0038,		// Format mask
	/** Symlink */
	FIO_SO_IFLNK		= 0x0008,		// Symbolic link
	/** Directory */
	FIO_SO_IFDIR		= 0x0010,		// Directory
	/** Regular file */
	FIO_SO_IFREG		= 0x0020,		// Regular file

	/** Hidden read permission */
	FIO_SO_IROTH		= 0x0004,		// read
	/** Hidden write permission */
	FIO_SO_IWOTH		= 0x0002,		// write
	/** Hidden execute permission */
	FIO_SO_IXOTH		= 0x0001,		// execute
};

// File mode checking macros
#define FIO_SO_ISLNK(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFLNK)
#define FIO_SO_ISREG(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFREG)
#define FIO_SO_ISDIR(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFDIR)

typedef int SceMode;
typedef long long SceOff;
typedef long long SceIores;

typedef struct ScePspDateTime {
	unsigned short	year;
	unsigned short 	month;
	unsigned short 	day;
	unsigned short	hour;
	unsigned short	minute;
	unsigned short	second;
	unsigned int	microsecond;
} ScePspDateTime;

#define PSP_CHSTAT_MODE   0x01
#define PSP_CHSTAT_ATTR   0x02
#define PSP_CHSTAT_SIZE   0x04
#define PSP_CHSTAT_CTIME  0x08
#define PSP_CHSTAT_ATIME  0x10
#define PSP_CHSTAT_MTIME  0x20

typedef struct SceIoStat 
{
	SceMode 		st_mode;
	unsigned int 	st_attr;
	/** Size of the file in bytes. */
	SceOff 			st_size;
	/** Creation time. */
	ScePspDateTime 	stctime;
	/** Access time. */
	ScePspDateTime 	statime;
	/** Modification time. */
	ScePspDateTime 	stmtime;
	/** Device-specific data. */
	unsigned int 	st_private[6];
} SceIoStat;

typedef struct SceIoDirent 
{
	/** File status. */
	SceIoStat 	d_stat;
	/** File name. */
	char 		d_name[256];
	/** Device-specific data. */
	void * 		d_private;
	int 		dummy;
} SceIoDirent;

#endif /*  __PSPIOFILEMGR_H__ */
