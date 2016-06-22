TOP := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

.DEFAULT_GOAL := all
sbindir=/usr/sbin
libdir=/usr/lib

LDLIBS+=$(shell pkg-config --libs $(PACKAGE_DEPS))
ALL_CFLAGS+=$(shell pkg-config --cflags $(PACKAGE_DEPS)) -fPIC -Werror $(CFLAGS)

INSTALLDEPS?=install-bins
BIN_SUFFIX?=.exe
DEFAULT_ALL?=$(BINS)

all: $(DEFAULT_ALL)

%.o: %.c
	$(CC) -c $(ALL_CFLAGS) -o $@ $<

$(BINS): %: %.o $(EXTRA_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@$(BIN_SUFFIX) $^ $(LDLIBS)

install-bins: $(BINS)
	@mkdir -p $(DESTDIR)$(sbindir)
	@for b in $(BINS); do \
		install $$b$(BIN_SUFFIX) $(DESTDIR)$(sbindir) || exit 1; \
	done

install: all $(INSTALLDEPS)

clean: $(CLEANDEPS)
	rm -rf *.o $(BINS:=$(BIN_SUFFIX))
