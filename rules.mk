TOP := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

.DEFAULT_GOAL := all
sbindir=/usr/sbin

LDLIBS+=$(shell pkg-config --libs $(PACKAGE_DEPS))
ALL_CFLAGS+=$(shell pkg-config --cflags $(PACKAGE_DEPS)) -fPIC -Werror $(CFLAGS)

BIN_SUFFIX?=.exe

all: $(BINS)

%.o: %.c
	$(CC) -c $(ALL_CFLAGS) -o $@ $<

$(BINS): %: %.o $(EXTRA_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@$(BIN_SUFFIX) $^ $(LDLIBS)

install: $(BINS)
	@mkdir -p $(DESTDIR)$(sbindir)
	@for b in $(BINS); do \
		install $$b$(BIN_SUFFIX) $(DESTDIR)$(sbindir) || exit 1; \
	done

clean:
	rm -rf *.o $(BINS:=$(BIN_SUFFIX))
