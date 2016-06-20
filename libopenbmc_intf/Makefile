PACKAGE_DEPS=gio-unix-2.0 glib-2.0
INSTALLDEPS=install-lib install-headers
CLEANDEPS=clean-lib
DEFAULT_ALL=$(LIBOBMC)
ALL_CFLAGS+=-iquote ../gdbus
LIBOBMC=openbmc_intf
INCLUDES=openbmc_intf.h openbmc.h gpio.h

$(LIBOBMC): %: %.o gpio.o
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -Wl,-soname,lib$(LIBOBMC).so \
		-o lib$@.so.1 $^ $(LDLIBS)

install-headers:
	@mkdir -p $(DESTDIR)$(includedir)
	install $(INCLUDES) $(DESTDIR)$(includedir)

install-lib:
	@mkdir -p $(DESTDIR)$(libdir)
	install lib$(LIBOBMC).so.1 $(DESTDIR)$(libdir)
	ln -s lib$(LIBOBMC).so.1 $(DESTDIR)$(libdir)/lib$(LIBOBMC).so

clean-lib:
	rm -f lib$(LIBOBMC).so.1

include ../rules.mk
