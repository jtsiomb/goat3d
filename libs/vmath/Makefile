csrc = $(wildcard *.c)
ccsrc = $(wildcard *.cc)
obj = $(csrc:.c=.o) $(ccsrc:.cc=.o)
lib = libvmath.a

ifneq ($(shell uname -s), Darwin)
	pic = -fPIC
endif

CFLAGS = -pedantic -Wall -g $(pic)
CXXFLAGS = -pedantic -Wall -g $(pic)

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
