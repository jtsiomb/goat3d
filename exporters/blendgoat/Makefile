name = blendgoat
scriptfile = blendgoat.py
srcpath := $(shell pwd)/src

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
		echo "installing to $(cfgpath)/$$i/scripts/addons/$(name)"; \
		rm -f $$i/scripts/addons/$(name)/*; \
		mkdir -p $$i/scripts/addons/$(name); \
		cp $(srcpath)/*.py $$i/scripts/addons/$(name)/; \
	done

.PHONY: link
link:
	@cd $(cfgpath) && \
	for i in *; do \
		echo "linking to $(cfgpath)/$$i/scripts/addons/$(name)"; \
		mkdir -p $$i/scripts/addons; \
		rm -rf $$i/scripts/addons/$(name); \
		ln -s $(srcpath) $$i/scripts/addons/$(name); \
	done

.PHONY: uninstall
uninstall:
	cd $(cfgpath) && \
	for i in *; do \
		rm -rf $$i/scripts/addons/$(name); \
	done
