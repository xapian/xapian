all: simplesearch simpleindex

simplesearch: simplesearch.cc
	$(CXX) $< `libomus-config --libs --cflags` -o $@

simpleindex: simpleindex.cc
	$(CXX) $< `libomus-config --libs --cflags` -o $@

clean:
	rm -f simpleindex simplesearch
