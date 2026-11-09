# BeAE - top level Makefile
#
# Running plain `make` here builds everything into ./BeAE:
#
# Handy targets:
#   make
#   make all             build everything
#   make app             build app
#   make clean           remove object files and the program
#   make help            print this list

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
