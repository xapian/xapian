## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT =

## Test programs to be run
TESTS = SmokeTest.exe

EXTRA_DIST = util.i AssemblyInfo.cs.in SmokeTest.cs $(BUILT_SOURCES)

SUBDIRS = docs

ASSEMBLY=XapianSharp

XAPIAN_SWIG_CS_SRCS=\
	Auto.cs \
	BM25Weight.cs \
	BoolWeight.cs \
	Database.cs \
	Document.cs \
	ESet.cs \
	ESetIterator.cs \
	Enquire.cs \
	InMemory.cs \
	MSet.cs \
	MSetIterator.cs \
	Muscat36.cs \
	PositionIterator.cs \
	PostingIterator.cs \
	Quartz.cs \
	Query.cs \
	QueryParser.cs \
	Remote.cs \
	RSet.cs \
	SWIGTYPE_p_Xapian__ExpandDecider.cs \
	SWIGTYPE_p_Xapian__MatchDecider.cs \
	SWIGTYPE_p_std__vector_Tstd__string_t.cs \
	SWIGTYPE_p_std__vector_TXapian__Query_t.cs \
	SWIGTYPE_p_time_t.cs \
	SimpleStopper.cs \
	Stem.cs \
	Stopper.cs \
	TermIterator.cs \
	TradWeight.cs \
	ValueIterator.cs \
	Weight.cs \
	WritableDatabase.cs \
	Xapian.cs \
	XapianPINVOKE.cs

XapianSharp.snk:
	$(SN) -k $@

$(ASSEMBLY).dll: $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs XapianSharp.snk
	$(CSC) -unsafe -target:library -out:$(ASSEMBLY).dll $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs

BUILT_SOURCES = XapianSharp.cc $(XAPIAN_SWIG_CS_SRCS)

if MAINTAINER_MODE
XapianSharp.cc $(XAPIAN_SWIG_CS_SRCS): ../xapian.i util.i
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f $(XAPIAN_SWIG_CS_SRCS)
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -csharp -namespace Xapian -module Xapian -dllimport XapianSharp -o XapianSharp.cc $(srcdir)/../xapian.i
	: # SWIG 1.3.27 names these differently to SWIG CVS HEAD, so check and
	: # rename to the CVS HEAD names so we can use either.
	test -f SWIGTYPE_p_std__vector_Tstd__string_t.cs || mv SWIGTYPE_p_std__vectorTstd__string_t.cs SWIGTYPE_p_std__vector_Tstd__string_t.cs
	test -f SWIGTYPE_p_std__vector_TXapian__Query_t.cs || mv SWIGTYPE_p_std__vectorTXapian__Query_t.cs SWIGTYPE_p_std__vector_TXapian__Query_t.cs
endif

lib_LTLIBRARIES = XapianSharp.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
XapianSharp_la_LDFLAGS = -avoid-version -module -no-undefined
nodist_XapianSharp_la_SOURCES = XapianSharp.cc
XapianSharp_la_LIBADD = $(XAPIAN_LIBS)

install-data-local: $(ASSEMBLY).dll
	$(GACUTIL) /i $(ASSEMBLY).dll /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

uninstall-local:
	$(GACUTIL) /u $(ASSEMBLY) /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

SmokeTest.exe: SmokeTest.cs $(ASSEMBLY).dll
	$(CSC) -unsafe -target:exe SmokeTest.cs -r:$(ASSEMBLY).dll

all-am: $(ASSEMBLY).dll

CLEANFILES = XapianSharp.snk AssemblyInfo.cs $(ASSEMBLY).dll SmokeTest.exe

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
