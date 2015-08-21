TARGET = cfe_loader
OBJS = main.o snprintf.o

# Use the kernel's small inbuilt libc
USE_KERNEL_LIBC = 1
# Use only kernel libraries
USE_KERNEL_LIBS = 1

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Werror -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ../lib

LIBS =
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
