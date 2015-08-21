ifdef LIGHT
GAME = 1
endif

ifdef GAME
ifndef LIGHT
TARGET = cfe_game
GUI_OBJS += gui/menu_music.o gui/filer.o
else
TARGET = cfe_light
endif
endif

ifdef VSH
TARGET = cfe_vsh
GUI_OBJS += gui/menu_tmachine.o gui/menu_music.o gui/filer.o
endif

ifndef LIGHT
MUSIC_OBJS = music/music.o music/init.o music/hw.o music/hw_mp3.o \
		music/hw_at3.o music/hw_aa3.o
else
MUSIC_OBJS = 
endif

GUI_OBJS += gui/guicommon.o gui/menu_info.o gui/menu_main.o \
		gui/menu_cfg.o


OBJS = main.o blit.o scr_printf.o mem.o utils.o snprintf.o screenshot.o conf.o ctrl.o \
	translateButtons.o graphic.o scePower_driver.o exports.o cfe_redirect.o \
	$(MUSIC_OBJS) $(GUI_OBJS)

BUILD_PRX=1
USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

ifdef VSH
CFLAGS += -DVSH
endif
ifdef GAME
CFLAGS += -DGAME
endif
ifdef LIGHT
CFLAGS += -DLIGHT
endif

CFLAGS += -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf -D_PSPSLIM \
	-DF_remotejoy_0001 -DF_remotejoy_0000 -DF_cfe_redirect_0000 -DF_cfe_redirect_0001
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

INCDIR = ../include ./ gui music
LIBDIR = ../lib
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpspkubridge -lpspsysmem_kernel -lpspsystemctrl_kernel -lpspusb_driver -lpsppower -lpspusbstor \
		-lpspumd -lpsputility -lpspaudiolib -lpspaudio -lpspaudiocodec -lpspkernel -lm -lpsplibc

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
