obj = main.o
bin = goatprim

goat_root = ../..

ifeq ($(shell uname -s), Darwin)
	libgoat = $(goat_root)/libgoat3d.dylib
else
	libgoat = $(goat_root)/libgoat3d.so.0.1
endif

CFLAGS = -pedantic -Wall -g -I$(goat_root)/src
LDFLAGS = -Wl,-rpath,$(goat_root) $(libgoat) -ltreestore

$(bin): $(obj) $(libgoat)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
