all: simplesearch simpleindex

simplesearch: simplesearch.cc
	$(CXX) $< `libomus-config --libs --cflags` -o $@

simpleindex: simpleindex.cc
	$(CXX) $< `libomus-config --libs --cflags` -o $@

html: simpleindex.cc.html simplesearch.cc.html

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
	rm -f simpleindex simplesearch
