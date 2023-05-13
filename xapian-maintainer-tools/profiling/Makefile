CC = gcc
CFLAGS = -Wall -W -O2
LIBS = -ldl

all: profiler.so

profiler.so: profiler.c
	$(CC) $(CFLAGS) -shared -fPIC -o profiler.so profiler.c $(LIBS)

clean:
	rm profiler.so

.PHONY: all clean
