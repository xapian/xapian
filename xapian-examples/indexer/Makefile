all: indexfile

indexfile: indexfile.cc
	$(CXX) $< `xapian-config --libs --cflags` -o $@

clean:
	rm -f indexfile
