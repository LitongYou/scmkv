TOP := all clean
SUBDIRS = src test

.PHONY:  $(TOP) $(SUBDIRS)

$(TOP): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)


