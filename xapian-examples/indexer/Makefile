all: indexfile

indexfile: indexfile.cc
	$(CXX) $< `libomus-config --libs --cflags` -o $@

clean:
	rm -f indexfile
