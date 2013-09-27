src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = goatview

goat_root = ..
libgoat = $(goat_root)/libgoat3d.so.0.1

CC = clang
CPP = clang -E
CFLAGS = -pedantic -Wall -g -I$(goat_root)/src
LDFLAGS = $(libgoat) -Wl,-rpath=$(goat_root) $(libgl)

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT -lGLEW
else
	libgl = -lGL -lGLU -lglut -lGLEW
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
