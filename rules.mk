.DEFAULT_GOAL := all
sbindir=/usr/sbin

LDLIBS+=$(shell pkg-config --libs $(PACKAGE_DEPS))
ALL_CFLAGS+=$(shell pkg-config --cflags $(PACKAGE_DEPS)) -fPIC -Werror $(CFLAGS)

BIN_SUFFIX?=.exe

all: $(BINS:=$(BIN_SUFFIX))

%.o: %.c
	$(CC) -c $(ALL_CFLAGS) -o $@ $<

$(BINS:=$(BIN_SUFFIX)): %$(BIN_SUFFIX): %.o $(EXTRA_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

install: $(BINS:=$(BIN_SUFFIX))
	@mkdir -p $(DESTDIR)$(sbindir)
	@for b in $(BINS:=$(BIN_SUFFIX)); do \
		install $$b $(DESTDIR)$(sbindir) || exit 1; \
	done

clean:
	rm -rf *.o $(BINS:=$(BIN_SUFFIX))
