sbin_PROGRAMS = phosphor-checkstop-handler

phosphor_checkstop_handler_SOURCES = \
				argument.cpp \
				checkstop.cpp \
				mainapp.cpp

phosphor_checkstop_handler_LDFLAGS = $(SYSTEMD_LIBS) $(PHOSPHOR_LOGGING_LIBS)
phosphor_checkstop_handler_CFLAGS = $(SYSTEMD_CFLAGS) $(PHOSPHOR_LOGGING_CFLAGS)
