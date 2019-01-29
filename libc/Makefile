all:
	rm -rf i686-woot
	$(MAKE) -C build
	$(MAKE) -C build install

install:
	$(MAKE) -C build install

clean:
	rm -rf i686-woot
	#$(MAKE) -C build clean # commented out so that 'rebuild all' in qtcreator doesn't make newlib to rebuild

.PHONY: all clean

