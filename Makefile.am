VERSION = @PACKAGE_VERSION@
# wxkb_switch_LOCAL_INSTALL = 0

bin_PROGRAMS = wxkb_switch
wxkb_switch_SOURCES = src/main.c src/xdg-shell-protocol.c src/shm.c


wxkb_switch_CFLAGS = $(DEPS_CFLAGS)
wxkb_switch_LDADD = $(DEPS_LIBS)

# bin_SCRIPTS = scripts/openswitcher.sh
# EXTRA_DIST = scripts/openswitcher.sh.in

# bin_SCRIPTS += scripts/input-device-info.sh
# EXTRA_DIST += scripts/input-device-info.sh.in

# CLEANFILES = $(bin_SCRIPTS)

man1_MANS = doc/wxkb_switch.1


# EXTRA_DIST += xkb-switch

# EXTRA_DIST += actkbd

# EXTRA_DIST += debian


# install-exec-hook: $(bin_PROGRAMS)
# 	@if [ $(OPENSWITCHER_LOCAL_INSTALL) = 1 ]; then\
# 		scripts/install.sh;\
# 	fi

# uninstall-hook:
# 	@if [ $(OPENSWITCHER_LOCAL_INSTALL) = 1 ]; then\
# 		scripts/uninstall.sh;\
# 	fi

# # compile xkb-switch + actkbd
# all-local:
# 	cd xkb-switch/ && mkdir -p build && cd build && cmake .. && $(MAKE)
# 	cd actkbd/ && $(MAKE)

# # clean xkb-switch + actkbd
# clean-local:
# 	rm -rf xkb-switch/build
# 	$(MAKE) --directory=actkbd clean

# distclean-local: clean
# 	rm -rf openswitcher*
# 	rm -rf doc/html
# 	rm -rf doc/docbook
# 	rm -rf doc/searchdata.xml

# $(PACKAGE_TARNAME)-$(VERSION).tar.gz: $(bin_PROGRAMS)
# 	$(MAKE) dist

# # make deb packege
# deb: $(PACKAGE_TARNAME)-$(VERSION).tar.gz
# 	cp $(PACKAGE_TARNAME)-$(VERSION).tar.gz $(PACKAGE_TARNAME)_$(VERSION).orig.tar.gz
# 	mkdir -p $(PWD)/$(PACKAGE_TARNAME)-$(VERSION)
# 	tar -xzf $(PACKAGE_TARNAME)-$(VERSION).tar.gz
# 	cd $(PACKAGE_TARNAME)-$(VERSION) && pwd && debuild
