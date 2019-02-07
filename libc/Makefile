INSTALL_DEPS = build/i686-woot/newlib/crt0.o build/i686-woot/newlib/libc.la build/i686-woot/newlib/libm.la

all:
	@$(MAKE) -C build
	#@$(MAKE) -C build install
	@$(MAKE) install.stamp

$(INSTALL_DEPS):

install.stamp: $(INSTALL_DEPS)
	@touch install.stamp
	@$(MAKE) -C build install

clean:
	@rm -rf i686-woot install.stamp
	#$(MAKE) -C build clean # commented out so that 'rebuild all' in qtcreator doesn't make newlib to rebuild

.PHONY: all clean

