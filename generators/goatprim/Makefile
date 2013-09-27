obj = main.o
bin = goatprim

goat_root = ../..

ifeq ($(shell uname -s), Darwin)
	libgoat = $(goat_root)/libgoat3d.dylib
else
	libgoat = $(goat_root)/libgoat3d.so.0.1 -Wl,-rpath=$(goat_root)
endif

CC = clang
CXX = clang++
CFLAGS = -pedantic -Wall -g -I$(goat_root)/src
LDFLAGS = $(libgoat)

$(bin): $(obj) $(libgoat)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
