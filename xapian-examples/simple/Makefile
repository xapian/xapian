all: simplecreate simpleindex simplesearch simpleexpand indexnasty

indexnasty: indexnasty.cc
	$(CXX) $< `xapian-config --libs --cxxflags` -o $@

simplecreate: simplecreate.cc
	$(CXX) $< `xapian-config --libs --cxxflags` -o $@

simpleindex: simpleindex.cc
	$(CXX) $< `xapian-config --libs --cxxflags` -o $@

simplesearch: simplesearch.cc
	$(CXX) $< `xapian-config --libs --cxxflags` -o $@

simpleexpand: simpleexpand.cc
	$(CXX) $< `xapian-config --libs --cxxflags` -o $@

html: simplecreate.cc.html simpleindex.cc.html \
      simplesearch.cc.html simpleexpand.cc.html

# This is the best way I've found to generate the html versions of the code
# on my system.  It's not very portable, but the result of its execution is
# available in the documentation for Xapian anyway.
%.cc.html: %.cc
	gvim -f +"highlight normal guibg=white" \
	        +"highlight normal guifg=black" \
		+"set background=light" \
		+"syn on" \
		+"so \$$VIMRUNTIME/syntax/2html.vim" \
		+"wq" \
		+"q" $<

clean:
	rm -f simplecreate simpleindex simplesearch simpleexpand indexnasty \
	      simplecreate.cc.html simpleindex.cc.html \
	      simplesearch.cc.html simpleexpand.cc.html
