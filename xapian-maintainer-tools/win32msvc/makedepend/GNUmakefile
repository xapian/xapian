# Makefile to allow at compilation to be tested with GCC on Linux.
# Call it GNUmakefile in case the presence of Makefile confuses nmake.

# Don't bother with dependencies, just always compile everything.
.PHONY: all
all:
	gcc -Wall -W -O2 -DSTANDALONE -DOBJSUFFIX=\".o\" *.c -o makedepend
