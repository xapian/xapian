all: indexfile

indexfile: indexfile.cc
	$(CXX) $< `omseek-config --libs --cflags` -o $@

clean:
	rm -f indexfile
