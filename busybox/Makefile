BB_DIR = busybox-1.29.3
BUILD_DIR = build
KMAKEFILE = $(BUILD_DIR)/Makefile
ROOT_DIR = $(shell pwd)

all: config
	$(MAKE) -C $(BUILD_DIR)

$(KMAKEFILE):
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) allnoconfig KBUILD_SRC=$(ROOT_DIR)/$(BB_DIR) -f $(ROOT_DIR)/$(BB_DIR)/Makefile
	
config: $(KMAKEFILE)

dist-clean:
	rm -rf $(BUILD_DIR)

clean:
	$(MAKE) -C $(BUILD_DIR) clean

.PHONY: all clean dist-clean

