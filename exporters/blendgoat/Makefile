scriptfile = blendgoat.py
scriptfile_path := $(shell pwd)/src/$(scriptfile)

ifeq ($(shell uname -s), Darwin)
	cfgpath = $(HOME)/Library/Application\ Support/Blender
else
	cfgpath = $(HOME)/.config/blender/
endif

.PHONY: all
all:
	@echo Just type \"$(MAKE) install\" to install the blender exporter.
	@echo Also make sure you have installed the main libgoat3d library.

.PHONY: install
install:
	@cd $(cfgpath) && \
	for i in *; do \
		echo "installing src/$(scriptfile) -> $(cfgpath)/$$i/scripts/addons/$(scriptfile)"; \
		mkdir -p $$i/scripts/addons; \
		rm -f $$i/scripts/addons/$(scriptfile); \
		cp $(scriptfile_path) $$i/scripts/addons/$(scriptfile); \
	done

.PHONY: link
link:
	cd $(cfgpath) && \
	for i in *; do \
		echo "linking src/$(scriptfile) to $(cfgpath)/$$i/scripts/addons/$(scriptfile)"; \
		mkdir -p $$i/scripts/addons; \
		rm -f $$i/scripts/addons/$(scriptfile); \
		ln -s $(scriptfile_path) $$i/scripts/addons/$(scriptfile); \
	done
