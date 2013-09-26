src = $(wildcard *.c) $(wildcard liblzma/*.c)
obj = $(src:.c=.o)
lib = libopenctm.a

ifneq ($(shell uname -s), Darwin)
	pic = -fPIC
endif

CFLAGS = -g -Iliblzma $(pic)

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
