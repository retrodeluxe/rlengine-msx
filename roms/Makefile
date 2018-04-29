SUBDIRS := $(wildcard */Makefile)

all clean: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $(dir $@) $(MAKECMDGOALS)

.PHONY: all clean $(SUBDIRS)
