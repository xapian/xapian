all: simplesearch simpleindex

simplesearch: simplesearch.cc
	$(CXX) `libomus-config --cflags --libs` -o $@ $<

simpleindex: simpleindex.cc
	$(CXX) `libomus-config --cflags --libs` -o $@ $<
