noinst_HEADERS +=\
	unicode/description_append.h

EXTRA_DIST +=\
	unicode/dir_contents\
	unicode/Makefile\
	unicode/UnicodeData-README.txt\
	unicode/UnicodeData.txt\
	unicode/uniParse.tcl

if MAINTAINER_MODE
unicode/tclUniData.cc: unicode/uniParse.tcl unicode/UnicodeData.txt
	rm -f unicode/UnicodeData-6.3.0.txt
	ln -s $(abs_srcdir)/unicode/UnicodeData.txt unicode/UnicodeData-6.3.0.txt
	tclsh $(srcdir)/unicode/uniParse.tcl $(srcdir)/unicode/UnicodeData-6.3.0.txt unicode

BUILT_SOURCES += unicode/tclUniData.cc
endif

lib_src +=\
	unicode/description_append.cc\
	unicode/tclUniData.cc\
	unicode/utf8itor.cc
