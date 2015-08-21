/*
 *	wifihost.h is part of host2ms
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

#pragma once

#include "conf.h"

#define CHALLENGE_TEXT_LEN	8

enum
{
	NET_HOSTFS_CMD_HELLO	= 0x00DA5800,
	NET_HOSTFS_CMD_IOINIT	= 0x00DA5801,
	NET_HOSTFS_CMD_IOEXIT	= 0x00DA5802,
	NET_HOSTFS_CMD_IOOPEN	= 0x00DA5803,
	NET_HOSTFS_CMD_IOCLOSE	= 0x00DA5804,
	NET_HOSTFS_CMD_IOREAD	= 0x00DA5805,
	NET_HOSTFS_CMD_IOWRITE	= 0x00DA5806,
	NET_HOSTFS_CMD_IOLSEEK	= 0x00DA5807,
	NET_HOSTFS_CMD_IOIOCTL	= 0x00DA5808,
	NET_HOSTFS_CMD_IOREMOVE	= 0x00DA5809,
	NET_HOSTFS_CMD_IOMKDIR	= 0x00DA580A,
	NET_HOSTFS_CMD_IORMDIR	= 0x00DA580B,
	NET_HOSTFS_CMD_IODOPEN	= 0x00DA580C,
	NET_HOSTFS_CMD_IODCLOSE	= 0x00DA580D,
	NET_HOSTFS_CMD_IODREAD	= 0x00DA580E,
	NET_HOSTFS_CMD_IOGETSTAT= 0x00DA580F,
	NET_HOSTFS_CMD_IOCHSTAT	= 0x00DA5810,
	NET_HOSTFS_CMD_IORENAME	= 0x00DA5811,
	NET_HOSTFS_CMD_IODEVCTL	= 0x00DA5812,
} NetHostFSCommand;

enum
{
	S_TERM = 10,
	S_SEND,
	S_RECV,
} SockOperation;

typedef struct SocketOpts
{
	int busy;
	int server;
	int thid;
	int sema;
	int operation;
	int res;
	int length;
	int * buf;
} SocketOpts;

typedef struct WifiCtrlOpts
{
	int inited;
	int thid;
	SocketOpts sock[4];
} WifiCtrlOpts;

typedef struct WifiOpenParams
{
	int cmd;
	char file[256];
	unsigned int fs_num; // Ignored at the moment
	int	flags;
	SceMode	mode;
} __attribute__((packed)) WifiOpenParams;

typedef struct WifiCloseParams
{
	int cmd;
	int fd;
} __attribute__((packed)) WifiCloseParams;

typedef struct WifiReadParams
{
	int cmd;
	int fd;
	int len;
} __attribute__((packed)) WifiReadParams;

typedef WifiReadParams WifiWriteParams;

typedef struct WifiLseekParams
{
	int cmd;
	int fd;
	SceOff offset;
	int whence;
} __attribute__((packed)) WifiLseekParams;

typedef struct WifiRemoveParams
{
	int cmd;
	char file[256];
	unsigned int fs_num; // Ignored at the moment
} __attribute__((packed)) WifiRemoveParams;

typedef struct WifiMkdirParams
{
	int cmd;
	char dir[256];
	unsigned int fs_num; // Ignored at the moment
	int mode;
} __attribute__((packed)) WifiMkdirParams;

typedef struct WifiRmdirParams
{
	int cmd;
	char dir[256];
	unsigned int fs_num; // Ignored at the moment
} __attribute__((packed)) WifiRmdirParams;

typedef WifiRmdirParams WifiDopenParams;

typedef WifiCloseParams WifiDcloseParams;

typedef WifiCloseParams WifiDreadParams;

typedef struct WifiDreadResult
{
	int res;
	SceIoDirent entry;
} __attribute__((packed)) WifiDreadResult;

typedef WifiRemoveParams WifiGetstatParams;

typedef struct WifiGetstatResult
{
	int res;
	SceIoStat stat;
} __attribute__((packed)) WifiGetstatResult;

typedef struct WifiChstatParams
{
	int cmd;
	char file[256];
	int fs_num;
	SceIoStat stat;
	int bits;
} __attribute__((packed)) WifiChstatParams;

typedef struct WifiRenameParams
{
	int cmd;
	char oldfile[256];
	char newfile[256];
	int fs_num;
} __attribute__((packed)) WifiRenameParams;

extern int startWifiHost( const char * name, HostCoreConf * config );

extern int stopWifiHost();
