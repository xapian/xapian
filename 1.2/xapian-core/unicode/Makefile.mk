EXTRA_DIST +=\
	unicode/dir_contents\
	unicode/Makefile\
	unicode/UnicodeData-README.txt\
	unicode/UnicodeData.txt\
	unicode/uniParse.tcl

if MAINTAINER_MODE
unicode/unicode-data.cc: unicode/uniParse.tcl unicode/UnicodeData.txt
	tclsh $(srcdir)/unicode/uniParse.tcl $(srcdir)/unicode/UnicodeData.txt 5.2.0 unicode/unicode-data.cc

BUILT_SOURCES += unicode/unicode-data.cc
MAINTAINERCLEANFILES += unicode/unicode-data.cc
endif

lib_src +=\
	unicode/unicode-data.cc\
	unicode/utf8itor.cc
