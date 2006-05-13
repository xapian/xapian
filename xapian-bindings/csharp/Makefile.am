## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = $(RUN_CSHARP)

## Test programs to be run
TESTS = SmokeTest.exe

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

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
	SWIGTYPE_p_std__vectorTstd__string_t.cs \
	SWIGTYPE_p_std__vectorTXapian__Query_t.cs \
	SWIGTYPE_p_time_t.cs \
	SimpleStopper.cs \
	Stem.cs \
	Stopper.cs \
	TermIterator.cs \
	TradWeight.cs \
	ValueIterator.cs \
	Version.cs \
	Weight.cs \
	WritableDatabase.cs \
	Xapian.cs \
	XapianPINVOKE.cs

XapianSharp.snk:
	$(SN) -k $@

$(ASSEMBLY).dll: $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs XapianSharp.snk
	$(CSC) -unsafe -target:library -out:$(ASSEMBLY).dll \
	    `for f in $(XAPIAN_SWIG_CS_SRCS); do if test -f "$$f"; then echo $$f; else echo $(srcdir)/$$f ; fi ; done` \
	    AssemblyInfo.cs

BUILT_SOURCES = xapian_wrap.cc $(XAPIAN_SWIG_CS_SRCS)

if MAINTAINER_MODE
xapian_wrap.cc $(XAPIAN_SWIG_CS_SRCS): ../xapian.i util.i
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f $(XAPIAN_SWIG_CS_SRCS)
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -csharp -namespace Xapian -module Xapian -dllimport XapianSharp -o xapian_wrap.cc $(srcdir)/../xapian.i
endif

lib_LTLIBRARIES = XapianSharp.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
XapianSharp_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_XapianSharp_la_SOURCES = xapian_wrap.cc
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
