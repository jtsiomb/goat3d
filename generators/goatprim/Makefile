obj = main.o
bin = goatprim

goat_root = ../..
libgoat = $(goat_root)/libgoat3d.so.0.1

CC = clang
CXX = clang++
CFLAGS = -pedantic -Wall -g -I$(goat_root)/src
LDFLAGS = $(libgoat) -Wl,-rpath=$(goat_root)

$(bin): $(obj) $(libgoat)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
