sbin_PROGRAMS = phosphor-checkstop-handler

noinst_HEADERS = checkstop.hpp

phosphor_checkstop_handler_SOURCES = \
				argument.cpp \
				checkstop.cpp \
				mainapp.cpp

phosphor_checkstop_handler_LDFLAGS = $(SYSTEMD_LIBS) \
									 $(SDBUSPLUS_LIBS) \
									 $(PHOSPHOR_LOGGING_LIBS) \
									 $(PHOSPHOR_DBUS_INTERFACES_LIBS)
phosphor_checkstop_handler_CFLAGS = $(SYSTEMD_CFLAGS) \
									$(SDBUSPLUS_CFLAGS) \
									$(PHOSPHOR_LOGGING_CFLAGS) \
									$(PHOSPHOR_DBUS_INTERFACES_CFLAGS)
