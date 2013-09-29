src = $(wildcard *.c)
obj = $(src:.c=.o)
lib = libanim.a

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
