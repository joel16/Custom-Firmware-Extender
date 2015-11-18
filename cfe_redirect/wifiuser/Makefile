#
#	This file is part of wifiuser
#	Copyright (C) 2008  Poison
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#	File:		Makefile
#	Author:		Poison <hbpoison@gmail.com>
#	Date Created:	2008-04-07

TARGET			= wifiuser
OBJS			= main.o ../log.o

PSP_FW_VERSION	= 660
BUILD_PRX		= 1
USE_PSPSDK_LIBC	= 1
USE_PSPSDK_LIBS = 1
PRX_EXPORTS		= exports.exp

INCDIR			=
LIBDIR			=
LDFLAGS			= -mno-crt0 -nostartfiles
LIBS			= -lpsputility -lpspnet -lpspnet_inet -lpspnet_resolver -lpspnet_apctl
CFLAGS			= -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf -D_DEBUG=0
CXXFLAGS		= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS			= $(CFLAGS)

PSPSDK			= $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
