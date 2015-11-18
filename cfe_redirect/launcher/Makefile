#
#	This file is part of launcher
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
#	Date Created:	2008-05-09

TARGET			= launcher
SRCS			= main.c
OBJS			= $(SRCS:.c=.o)

PSP_FW_VERSION	= 660
BUILD_PRX		= 1

INCDIR			= 
LIBDIR			=
LDFLAGS			=
LIBS			=
CFLAGS			= -O2 -G0 -Wall
CXXFLAGS		= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS			= $(CFLAGS)

EXTRA_TARGETS	= EBOOT.PBP
PSP_EBOOT_TITLE	= cfe_launcher
PSP_EBOOT_ICON 	=

PSPSDK			= $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
