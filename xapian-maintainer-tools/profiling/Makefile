CC = gcc
CFLAGS = -Wall -W -O2
LIBS = -ldl

all: profiler.so

profiler.so: profiler.c
	$(CC) $(CFLAGS) -shared -fPIC -o profiler.so profiler.c $(LIBS)

clean:
	rm profiler.so

# Check that we handle 64-bit file offsets suitably by running unittest
# testcase ioblock1.
#
# Normalise the output by stripping paths from open() and then diff against
# the expected result.
check:
	test -x ../../xapian-core/tests/unittest
	./xapian-io-profile ../../xapian-core/tests/unittest ioblock1 2>&1 >/dev/null |\
	    sed 's,^open(.*/,open(,' | diff - check.expect
	@echo "Tests passed"

.PHONY: all clean
