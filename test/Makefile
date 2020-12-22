SUBDIRS := $(wildcard */Makefile)

all clean_rom: $(SUBDIRS)

$(SUBDIRS):
	$(hide) $(MAKE) -C $(dir $@) $(MAKECMDGOALS)

.PHONY: all clean_rom $(SUBDIRS)
