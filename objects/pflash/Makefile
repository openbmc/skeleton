	ARCH_OBJS = arm_io.o

CFLAGS  = -O2 -Wall -I.
LDFLAGS	= -lrt 
OBJS    = pflash.o progress.o ast-sf-ctrl.o
OBJS	+= libflash/libflash.o libflash/libffs.o
OBJS	+= $(ARCH_OBJS)
EXE     = pflash

#CC	= $(CROSS_COMPILE)gcc

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I.

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -I.

clean:
	rm -f $(OBJS) $(EXE) *.o *.d libflash/test/test_flash libflash/test/*.o
distclean: clean
	rm -f *.c~ *.h~ *.sh~ Makefile~ config.mk~ libflash/*.c~ libflash/*.h~
