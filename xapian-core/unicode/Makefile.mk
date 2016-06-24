noinst_HEADERS +=\
	unicode/description_append.h

EXTRA_DIST +=\
	unicode/Makefile\
	unicode/c_istab.h\
	unicode/gen_c_istab\
	unicode/UnicodeData-README.txt\
	unicode/UnicodeData.txt\
	unicode/uniParse.tcl

if MAINTAINER_MODE
unicode/unicode-data.cc: unicode/uniParse.tcl unicode/UnicodeData.txt
	tclsh $(srcdir)/unicode/uniParse.tcl $(srcdir)/unicode/UnicodeData.txt 9.0.0 unicode/unicode-data.cc

unicode/c_istab.h: unicode/gen_c_istab
	$(PERL) $(srcdir)/unicode/gen_c_istab unicode/c_istab.h

BUILT_SOURCES += unicode/unicode-data.cc unicode/c_istab.h
MAINTAINERCLEANFILES += unicode/unicode-data.cc unicode/c_istab.h
endif

lib_src +=\
	unicode/description_append.cc\
	unicode/unicode-data.cc\
	unicode/utf8itor.cc
