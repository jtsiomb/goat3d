obj = tinyxml2.o
lib = libtinyxml2.a

ifneq ($(shell uname -s), Darwin)
	pic = -fPIC
endif

CXXFLAGS = -pedantic -Wall -g $(pic)

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
