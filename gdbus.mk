PACKAGE_DEPS=gio-unix-2.0 glib-2.0
ALL_CFLAGS+=-iquote ../gdbus -iquote ../libopenbmc_intf

LIBOBMC=$(TOP)/libopenbmc_intf/libopenbmc_intf.so.1
EXTRA_OBJS+=$(LIBOBMC)

$(LIBOBMC):
	$(MAKE) -C $(TOP)/libopenbmc_intf

%.o: %_obj.c
	$(CC) -c $(ALL_CFLAGS) -o $@ $<
