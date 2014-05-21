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
LDFLAGS = $(libgoat) $(libgl) $(qtlib) -lvmath -lanim
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
	libgoat = $(goat_root)/libgoat3d.dylib
	warn += -Wno-deprecated-declarations
	bundle = $(bin).app
	dmg = $(bin).dmg
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

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
	[ -n "$(bundle)" ] && rm -rf $(bundle) || true
	[ -n "$(dmg)" ] && rm -rf $(dmg) || true

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: echo
echo:
	@echo $(var)=\"$($(var))\"

.PHONY: bundle
bundle: $(bundle)

.PHONY: dmg
dmg: $(dmg)

$(bundle): $(bin) $(libgoat)
	rm -rf $(bundle)
	mkdir -p $(bundle)/Contents/MacOS
	mkdir -p $(bundle)/Contents/Frameworks
	cp $(bin) $(bundle)/Contents/MacOS/$(bin)
	macdeployqt $(bundle)
	cp $(libgoat) $(bundle)/Contents/Frameworks
	cp /usr/local/lib/libvmath.dylib $(bundle)/Contents/Frameworks
	cp /usr/local/lib/libanim.dylib $(bundle)/Contents/Frameworks

$(dmg): $(bundle)
	rm -f $@
	hdiutil create -format UDBZ -quiet -srcfolder $< $@

.PHONY: run
ifeq ($(shell uname -s), Darwin)
run: $(bundle)
	./$(bundle)/Contents/MacOS/$(bin)
else
run: $(bin)
	./$(bin)
endif

ccver = $(shell $(CC) --version)
ifneq (,$(findstring LLVM,$(ccver)))
	# clang/llvm
	compiler = clang
else
	# other, assume gcc
	compiler = gcc
endif
