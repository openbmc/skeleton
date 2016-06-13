BINS=flasher
EXTRA_OBJS+=progress.o \
	    ast-sf-ctrl.o \
	    libflash.o \
	    libffs.o \
	    arm_io.o
CFLAGS+=-Ipflash
include ../gdbus.mk
include ../rules.mk

%.o: pflash/%.c
	$(CC) -c -o $(CFLAGS) -fPIC -o $@ $<
%.o: pflash/libflash/%.c
	$(CC) -c -o $(CFLAGS) -fPIC -o $@ $<
