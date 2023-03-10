PREFIX = /usr/local

src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = ass2goat

goat_root = ../..
goatinc = -I$(goat_root)/src

CFLAGS = -pedantic -Wall -g $(goatinc) -MMD
LDFLAGS = -Wl,-rpath=$(goat_root) -L$(goat_root) -lgoat3d -lassimp -ltreestore

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
