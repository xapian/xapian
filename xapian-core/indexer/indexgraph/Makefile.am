## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS = deleter_map.h omindexerinternal.h omnodeconnection.h\
		register_core.h node_reg.h regexcommon.h indexerxml.h \
		toposort.h omnodedescriptorinternal.h

data_DATA = omindexer.dtd

noinst_LTLIBRARIES = libindexgraph.la

#noinst_PROGRAMS = nodetest
#
#nodetest_SOURCES = nodetest.cc
#
#nodetest_LDADD = ../../libomus.la

if HAVE_REGEX
REGEX_NODES = omregexreplacenode.cc omregexsplitnode.cc omregexfilternode.cc
REGEX_LIBS = @REGEX_LIBS@
else
REGEX_NODES = 
REGEX_LIBS = 
endif

INDEXERNODES = omstemmernode.cc omsplitternode.cc omlistconcatnode.cc \
		omprefixnode.cc omstopwordnode.cc $(REGEX_NODES) \
		omflattenstringnode.cc omtranslatenode.cc \
		omfilereadernode.cc omvectorsplitnode.cc

libindexgraph_la_SOURCES = omindexer.cc omindexermessage.cc \
	do_registration.cc $(INDEXERNODES) indexerxml.cc \
	toposort.cc omindexerbuilder.cc omnodedescriptor.cc \
	omindexernode.cc

indexerxml.lo: indexerxml.cc
	$(LTCXXCOMPILE) @LIBXML_CFLAGS@ -c $<

do_registration.cc: $(INDEXERNODES) make_register.pl
	srcdir=$(srcdir) ./make_register.pl $(INDEXERNODES) > $@

libindexgraph_la_LIBADD = @LIBXML_LIBS@ $(REGEX_LIB)
