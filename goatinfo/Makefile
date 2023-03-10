PREFIX = /usr/local

obj = goatinfo.o
bin = goatinfo

goat_root = ..

CFLAGS = $(warn) -g -I$(goat_root)/src
LDFLAGS = -Wl,-rpath=$(goat_root) -L$(goat_root) -lgoat3d

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
