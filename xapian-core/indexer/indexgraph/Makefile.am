## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = deleter_map.h omindexerinternal.h \
		register_core.h node_reg.h regexcommon.h indexerxml.h \
		toposort.h omnodedescriptorinternal.h \
		ompaditeratorinternal.h omnodeinstanceiteratorinternal.h \
		omindexerdescinternal.h omnodepad.h

data_DATA = omindexer.dtd

EXTRA_DIST = omindexer.dtd

noinst_LTLIBRARIES = libindexgraph.la

#noinst_PROGRAMS = nodetest
#
#nodetest_SOURCES = nodetest.cc
#
#nodetest_LDADD = ../../libomseek.la

if HAVE_REGEX
REGEX_NODES = omregexreplacenode.cc omregexsplitnode.cc \
		omregexfilternode.cc omregexmatchnode.cc
REGEX_LIBS = @REGEX_LIBS@
else
REGEX_NODES = 
REGEX_LIBS = 
endif

INDEXERNODES = omstemmernode.cc omsplitternode.cc omlistconcatnode.cc \
		omprefixnode.cc omstopwordnode.cc $(REGEX_NODES) \
		omflattenstringnode.cc omtranslatenode.cc \
		omfilereadernode.cc omvectorsplitnode.cc \
		ommakedocnode.cc omconstantnode.cc omnewtermlistnode.cc \
		omtermlistaddnode.cc omnewkeylistnode.cc \
		omkeylistaddnode.cc ommakepairnode.cc \
		ommakepairsnode.cc ommakerangenode.cc \
		omselectitemsnode.cc omsplitoncharsnode.cc

libindexgraph_la_SOURCES = omindexer.cc omindexermessage.cc \
	do_registration.cc $(INDEXERNODES) indexerxml.cc \
	toposort.cc omindexerbuilder.cc omnodedescriptor.cc \
	omindexernode.cc ompaditerator.cc omindexerdesc.cc \
	omnodeinstanceiterator.cc

indexerxml.lo: indexerxml.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

indexerxml.o: indexerxml.cc
	$(CXXCOMPILE) @LIBXML_CFLAGS@ -c $<

do_registration.cc: $(INDEXERNODES) make_register.pl
	srcdir=$(srcdir) ./make_register.pl $(INDEXERNODES) > $@

libindexgraph_la_LIBADD = @LIBXML_LIBS@ $(REGEX_LIB)
