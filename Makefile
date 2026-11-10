# BeAE - top level Makefile
#
.PHONY: all
all:
	$(MAKE) -C Core-src
%:
	$(MAKE) -C Core-src $@
Makefile:
