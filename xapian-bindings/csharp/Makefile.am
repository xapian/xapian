## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT =

## Test programs to be run
TESTS = SmokeTest.exe

EXTRA_DIST = util.i extra.i AssemblyInfo.cs.in SmokeTest.cs $(BUILT_SOURCES)

ASSEMBLY=XapianSharp

XAPIAN_SWIG_CS_SRCS=\
	BM25Weight.cs \
	BoolWeight.cs \
	Database.cs \
	Document.cs \
	ESet.cs \
	ESetIterator.cs \
	Enquire.cs \
	MSet.cs \
	MSetIterator.cs \
	PositionIterator.cs \
	PostingIterator.cs \
	Query.cs \
	QueryParser.cs \
	RSet.cs \
	SWIGTYPE_p_Xapian__ExpandDecider.cs \
	SWIGTYPE_p_Xapian__MatchDecider.cs \
	SWIGTYPE_p_std__listTstd__string_t.cs \
	SWIGTYPE_p_time_t.cs \
	SWIGTYPE_p_vectorTstd__string_t.cs \
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
	$(CSC) --unsafe --target library -o $(ASSEMBLY).dll $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs

BUILT_SOURCES = $(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS)

if MAINTAINER_MODE
XapianSharp.cc $(XAPIAN_SWIG_CS_SRCS): ../xapian.i util.i extra.i
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -csharp -namespace Xapian -module Xapian -o $(ASSEMBLY).cc $(srcdir)/../xapian.i
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
	$(CSC) --unsafe --target exe SmokeTest.cs -r:$(ASSEMBLY).dll

all-am: $(ASSEMBLY).dll

CLEANFILES = XapianSharp.snk AssemblyInfo.cs $(ASSEMBLY).dll SmokeTest.exe

MAINTAINERCLEANFILES = $(BUILT_SOURCES)
