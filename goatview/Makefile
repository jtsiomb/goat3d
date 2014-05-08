PREFIX = /usr/local

src = $(wildcard src/*.cc)
mochdr = src/goatview.h
mocsrc = $(patsubst src/%.h,src/moc_%.cc,$(mochdr))
obj = $(sort $(src:.cc=.o) $(mocsrc:.cc=.o))
dep = $(obj:.o=.d)
bin = goatview

goat_root = ..

CXXFLAGS = -std=c++11 -pedantic -Wall -g $(pic) -I$(goat_root)/src $(qtinc)
LDFLAGS = $(libgoat) $(libgl) $(qtlib) -lvmath
MOC = moc

qtinc = `pkg-config --cflags Qt5Gui Qt5Core Qt5OpenGL`
qtlib = `pkg-config --libs Qt5Gui Qt5Core Qt5OpenGL`

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT -lGLEW
	libgoat = $(goat_root)/libgoat3d.dylib
else
	pic = -fPIC
	libgl = -lGL -lGLU -lglut -lGLEW
	libgoat = $(goat_root)/libgoat3d.so.0.1
endif

$(bin): $(obj) $(libgoat)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

# rule for running moc on ui header files
moc_%.cc: %.h
	$(MOC) -o $@ $<

-include $(dep)

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
