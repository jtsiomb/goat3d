# ----- options -----
PREFIX = /usr/local
dbg = -g
opt = -O0
# -------------------

src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
dep = $(obj:.o=.d)

openctm = libs/openctm/libopenctm.a
tinyxml2 = libs/tinyxml2/libtinyxml2.a
vmath = libs/vmath/libvmath.a
anim = libs/anim/libanim.a

extinc = -Ilibs/openctm -Ilibs/tinyxml2 -Ilibs/anim
extlibs = $(openctm) $(tinyxml2) $(anim) $(vmath)

name = goat3d
so_major = 0
so_minor = 1

lib_a = lib$(name).a

ifeq ($(shell uname -s), Darwin)
	lib_so = lib$(name).dylib
	shared = -dynamiclib
else
	devlink = lib$(name).so
	soname = lib$(name).so.$(so_major)
	lib_so = lib$(name).so.$(so_major).$(so_minor)

	shared = -shared -Wl,-soname=$(soname)
	pic = -fPIC
endif

CC = clang
CXX = clang++
CXXFLAGS = -pedantic -Wall $(dbg) $(opt) $(pic) $(extinc)
LDFLAGS = $(extlibs) -lpthread

.PHONY: all
all: $(lib_so) $(lib_a)

$(lib_so): $(obj) $(extlibs)
	$(CXX) -o $@ $(shared) $(obj) $(LDFLAGS)

$(lib_a): $(obj) $(extlibs)
	$(AR) rcs $@ $(obj) $(extlibs)

.PHONY: $(openctm)
$(openctm):
	$(MAKE) -C libs/openctm

.PHONY: $(tinyxml2)
$(tinyxml2):
	$(MAKE) -C libs/tinyxml2

.PHONY: $(vmath)
$(vmath):
	$(MAKE) -C libs/vmath

.PHONY: $(anim)
$(anim):
	$(MAKE) -C libs/anim

-include $(dep)

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

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
	[ -n "$(devlink)" ] && \
		cd $(DESTDIR)$(PREFIX)/lib && \
		rm -f $(soname) $(devlink) && \
		ln -s $(lib_so) $(soname) && \
		ln -s $(soname) $(devlink) || \
		true

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/include/goat3d.h
	rm -f $(DESTDIR)$(PREFIX)/lib/$(lib_so)
	rm -f $(DESTDIR)$(PREFIX)/lib/$(lib_a)
	[ -n "$(devlink)" ] && \
		rm -f $(DESTDIR)$(PREFIX)/lib/$(soname) && \
		rm -f $(DESTDIR)$(PREFIX)/lib/$(devlink) || \
		true
