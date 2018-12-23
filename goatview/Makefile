PREFIX = /usr/local

src = $(wildcard src/*.cc)
csrc = $(wildcard src/*.c)
mochdr = src/goatview.h src/scenemodel.h
mocsrc = $(patsubst src/%.h,src/moc_%.cc,$(mochdr))
obj = $(sort $(csrc:.c=.o) $(src:.cc=.o) $(mocsrc:.cc=.o))
dep = $(obj:.o=.d)
bin = goatview

goat_root = ..

warn = -Wall

CFLAGS = $(warn) -g $(pic) -I$(goat_root)/src $(qtinc)
CXXFLAGS = -std=c++11 $(CFLAGS)
LDFLAGS = -Wl,-rpath=$(goat_root) -L$(goat_root) -lgoat3d $(libgl) $(qtlib) -lvmath -lanim
MOC = moc

qtinc = `pkg-config --cflags Qt5Gui Qt5Core Qt5OpenGL`
qtlib = `pkg-config --libs Qt5Gui Qt5Core Qt5OpenGL`

ifeq ($(compiler), clang)
	CXXFLAGS += -stdlib=libc++
	warn += '-Wno-\#warnings'
else
	warn += -Wno-cpp
endif

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT -lGLEW
	warn += -Wno-deprecated-declarations
else
	pic = -fPIC
	libgl = -lGL -lGLU -lglut -lGLEW
endif

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

# rule for running moc on ui header files
moc_%.cc: %.h
	$(MOC) -o $@ $<

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
