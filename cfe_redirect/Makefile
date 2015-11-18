#
#	This file is part of host2ms
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
#	Date Created:	2008-04-01

TARGET			= cfe_redirect
OBJS			= main.o utils.o syspatch.o ctrl.o conf.o mem.o umd.o usbhost.o wifihost.o tinyui.o log.o

PSP_FW_VERSION	= 660
BUILD_PRX		= 1
USE_KERNEL_LIBC	= 1
USE_KERNEL_LIBS = 1

PRX_EXPORTS		= exports.exp

INCDIR			= 
LIBDIR			= 
LDFLAGS			= -mno-crt0 -nostartfiles
LIBS			= -lpspusb_driver -lpspreg_driver -lpspumd_driver
CFLAGS			= -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf -D_DEBUG=1
CXXFLAGS		= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS			= $(CFLAGS)

PSPSDK			= $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

clean-release:
	rm -rf HostCore_1.2.tar.gz
	rm -rf release

release: wifi usb launcher-eboot clean all clean-release
	mkdir release
	cp -r extras release/HostCore
	mv release/HostCore/readme.txt release/
	mv release/HostCore/readme.rtf release/
	cp hostcore.prx release/HostCore/
	cp usbhostfs/usbhostfs.prx release/HostCore/
	cp wifiuser/wifiuser.prx release/HostCore/
	cp launcher/EBOOT.PBP release/HostCore/launcher.pbp
	rm -rf release/HostCore/.svn
	rm -rf release/HostCore/resources/.svn
	find release -type f -name '.*' -exec rm -f {} \;
	find release -type d -name '.*' -exec rm -rf {} \;
	tar czf HostCore_1.2.tar.gz release/
	
launcher-eboot:
	$(MAKE) -C launcher clean
	$(MAKE) -C launcher all

wifi:
	$(MAKE) -C wifiuser clean
	$(MAKE) -C wifiuser all

usb:
	$(MAKE) -C usbhostfs clean
	$(MAKE) -C usbhostfs all

install:
	cp hostcore.prx /Volumes/NO\ NAME/HostCore/
	cp launcher/EBOOT.PBP /Volumes/NO\ NAME/HostCore/launcher.pbp
	cp usbhostfs/usbhostfs.prx /Volumes/NO\ NAME/HostCore/
	cp wifiuser/wifiuser.prx /Volumes/NO\ NAME/HostCore/
	rm -f /Volumes/NO\ NAME/log.txt
	rm -rf /Volumes/NO\ NAME/DUMP
	diskutil eject /Volumes/NO\ NAME/

vlog:
	mate /Volumes/NO\ NAME/log.txt
