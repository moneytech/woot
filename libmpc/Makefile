CONFIG_STATUS = build/config.status

all: config
	@$(MAKE) -C build
	@$(MAKE) -C build install

$(CONFIG_STATUS): ./configure
	@./configure

config: $(CONFIG_STATUS)

reconfig: dist-clean config

dist-clean:	
	@rm -rf build include lib share

clean:
	@$(MAKE) -C build clean

.PHONY: all reconfig dist-clean clean

