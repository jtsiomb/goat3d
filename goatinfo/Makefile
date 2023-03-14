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

.PHONY: install
install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
