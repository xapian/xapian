all: simplecreate simpleindex simplesearch simpleexpand

simplecreate: simplecreate.cc
	$(CXX) $< `omsee-config --libs --cflags` -o $@

simpleindex: simpleindex.cc
	$(CXX) $< `omsee-config --libs --cflags` -o $@

simplesearch: simplesearch.cc
	$(CXX) $< `omsee-config --libs --cflags` -o $@

simpleexpand: simpleexpand.cc
	$(CXX) $< `omsee-config --libs --cflags` -o $@

html: simplecreate.cc.html simpleindex.cc.html \
      simplesearch.cc.html simpleexpand.cc.html

# This is the best way I've found to generate the html versions of the code
# on my system.  It's not very portable, but the result of its execution is
# available in the documentation for Muscat anyway.
%.cc.html: %.cc
	gvim -f +"highlight normal guibg=white" \
	        +"highlight normal guifg=black" \
		+"set background=light" \
		+"syn on" \
		+"so \$$VIMRUNTIME/syntax/2html.vim" \
		+"wq" \
		+"q" $<

clean:
	rm -f simplecreate simpleindex simplesearch simpleexpand \
	      simplecreate.cc.html simpleindex.cc.html \
	      simplesearch.cc.html simpleexpand.cc.html
