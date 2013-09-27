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

extinc = -Ilibs/openctm -Ilibs/tinyxml2
extlibs = $(openctm) $(tinyxml2)

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
LDFLAGS = $(extlibs) -lvmath -lanim

.PHONY: all
all: $(lib_so) $(lib_a)

$(lib_so): $(obj) $(extlibs)
	$(CXX) -o $@ $(shared) $(obj) $(LDFLAGS)

$(lib_a): $(obj) $(extlibs)
	$(AR) rcs $@ $(obj) $(openctm)

$(openctm):
	$(MAKE) -C libs/openctm

$(tinyxml2):
	$(MAKE) -C libs/tinyxml2

-include $(dep)

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(lib_a) $(lib_so)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
