# BeAE - top level Makefile
#
# Running plain `make` here builds everything into ./BeAE:
#
# Each component keeps its own makefile (they all use the Haiku
# makefile-engine); this file only decides what gets built and where the
# results go.
#
# Handy targets:
#   make
#   make all             build everything
#   make app             build app
#   make clean           remove object files and the program
#   make help            print this list
#
# Only basic GNU make features are used so this works with the make that
# ships with the gcc2 (x86_gcc2) builds of Haiku as well as newer ones.
# The compiler itself is picked by the makefile-engine, so the usual
# setgcc / hybrid gcc2 setup decides what gets used.

BUILD_DIR   := $(CURDIR)/BeAE

.PHONY: all app clean help

all: app

app:
	@mkdir -p "$(BUILD_DIR)"
	$(MAKE) -C Core-src TARGET_DIR="$(BUILD_DIR)"

clean:
	$(MAKE) -C Core-src clean
	rm -f $(BUILD_DIR)/BeAE

help:
	@echo "Targets:"
	@echo "  all (default)   build the application"
	@echo "  app             build the application only"
	@echo "  clean           remove object files and the program"
	@echo "Results end up in $(BUILD_DIR)"
