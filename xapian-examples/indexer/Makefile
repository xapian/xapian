all: indexfile

indexfile: indexfile.cc
	$(CXX) $< `omsee-config --libs --cflags` -o $@

clean:
	rm -f indexfile
