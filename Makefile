# ----- options -----
#  TODO make configure script
PREFIX = /usr/local
dbg = -g
opt = -O0
# -------------------

src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)

name = goat3d
so_major = 0
so_minor = 1

lib_a = lib$(name).a

ifeq ($(shell uname -s), Darwin)
	lib_so = lib$(name).dylib
	shared = -dynamiclib
else
	ldname = lib$(name).so
	soname = lib$(name).so.$(so_major)
	lib_so = lib$(name).so.$(so_major).$(so_minor)

	shared = -shared -Wl,-soname=$(soname)
	pic = -fPIC
endif

CFLAGS = -pedantic -Wall $(dbg) $(opt) $(pic)
LDFLAGS = -lanim

.PHONY: all
all: $(lib_so) $(lib_a) $(soname) $(ldname)

$(lib_so): $(obj)
	$(CC) -o $@ $(shared) $(obj) $(LDFLAGS)

$(lib_a): $(obj)
	$(AR) rcs $@ $(obj)

$(soname): $(lib_so)
	rm -f $@
	ln -s $< $@

$(ldname): $(soname)
	rm -f $@
	ln -s $< $@

-include $(dep)

%.d: %.c
	@echo "generating depfile $@"
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(lib_a) $(lib_so)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(lib_so) $(lib_a)
	mkdir -p $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
	cp src/goat3d.h $(DESTDIR)$(PREFIX)/include/goat3d.h
	cp $(lib_a) $(DESTDIR)$(PREFIX)/lib/$(lib_a)
	cp $(lib_so) $(DESTDIR)$(PREFIX)/lib/$(lib_so)
	[ -n "$(ldname)" ] && \
		cd $(DESTDIR)$(PREFIX)/lib && \
		rm -f $(soname) $(ldname) && \
		ln -s $(lib_so) $(soname) && \
		ln -s $(soname) $(ldname) || \
		true

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/include/goat3d.h
	rm -f $(DESTDIR)$(PREFIX)/lib/$(lib_so)
	rm -f $(DESTDIR)$(PREFIX)/lib/$(lib_a)
	[ -n "$(ldname)" ] && \
		rm -f $(DESTDIR)$(PREFIX)/lib/$(soname) && \
		rm -f $(DESTDIR)$(PREFIX)/lib/$(ldname) || \
		true
