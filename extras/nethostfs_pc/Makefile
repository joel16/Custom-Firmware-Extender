OUTPUT=nethostfs
OBJS=main.o
LIBS=
CFLAGS=-Wall -I.
LDFLAGS=-L. -lpthread

ifdef __CYGWIN__
LIBS += -lwinmm
endif

ifdef BUILD_BIGENDIAN
CFLAGS += -DBUILD_BIGENDIAN=1
endif

all: $(OUTPUT)
	
install:
	cp $(OUTPUT) /usr/local/bin/nethostfs_pc

clean:
	rm -f $(OUTPUT) *.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)
