SUBDIRS := $(wildcard */Makefile)

all clean: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $(dir $@) $(MAKEFLAGS) $(MAKECMDGOALS)

.PHONY: all clean $(SUBDIRS)
