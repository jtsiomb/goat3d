src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = goatview

goat_root = ..

CC = clang
CPP = clang -E
CFLAGS = -pedantic -Wall -g -I$(goat_root)/src
LDFLAGS = $(libgoat) $(libgl)

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT -lGLEW
	libgoat = $(goat_root)/libgoat3d.dylib
else
	libgl = -lGL -lGLU -lglut -lGLEW
	libgoat = $(goat_root)/libgoat3d.so.0.1
endif

$(bin): $(obj) $(libgoat)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
