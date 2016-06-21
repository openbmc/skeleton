GDBUS_APPS = bmcctl \
	     flashbios \
	     hostcheckstop \
	     hostwatchdog \
	     op-flasher \
	     op-hostctl \
	     op-pwrctl \
	     pciedetect \
	     pwrbutton \
	     rstbutton

SUBDIRS = $(GDBUS_APPS) \
	  hacks \
	  ledctl \
	  libopenbmc_intf \
	  pychassisctl \
	  pydownloadmgr \
	  pyfanctl \
	  pyflashbmc \
	  pyhwmon \
	  pyinventorymgr \
	  pyipmitest \
	  pysensormgr \
	  pystatemgr \
	  pysystemmgr \
	  pytools

REVERSE_SUBDIRS = $(shell echo $(SUBDIRS) | tr ' ' '\n' | tac |tr '\n' ' ')

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(GDBUS_APPS): libopenbmc_intf

install: subdirs
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@ DESTDIR=$(DESTDIR) PREFIX=$(PREFIX) || exit 1; \
	done
clean:
	@for d in $(REVERSE_SUBDIRS); do \
		$(MAKE) -C $$d $@ || exit 1; \
	done
