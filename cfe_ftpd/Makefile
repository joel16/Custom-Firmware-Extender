
TARGET = cfe_ftpd
OBJS = main.o blit.o scr_printf.o ftp.o ftpd.o sutils.o conf.o psp_init.o nlh.o

INCDIR =
CFLAGS = -O2 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

BUILD_PRX = 1
#USE_PSPSDK_LIBC = 1

LIBDIR =
LIBS = -lpspwlan -lm
LDFLAGS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
