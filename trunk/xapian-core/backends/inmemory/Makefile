# Makefile for use in directories built by non-recursive make.

SHELL = /bin/sh

all check check-syntax:
	cd ../.. && $(MAKE) $@

clean:
	rm -f *.o *.obj *.lo
