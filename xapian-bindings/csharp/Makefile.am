## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = $(RUN_CSHARP)

## Test programs to be run
TESTS = SmokeTest.exe

SUBDIRS = docs

ASSEMBLY=XapianSharp

XAPIAN_SWIG_CS_SRCS=\
	Auto.cs \
	BM25Weight.cs \
	BoolWeight.cs \
	Database.cs \
	DateValueRangeProcessor.cs \
	Document.cs \
	ESet.cs \
	ESetIterator.cs \
	Enquire.cs \
	ExpandDecider.cs \
	Flint.cs \
	InMemory.cs \
	MatchDecider.cs \
	MSet.cs \
	MSetIterator.cs \
	NumberValueRangeProcessor.cs \
	PositionIterator.cs \
	PostingIterator.cs \
	Quartz.cs \
	Query.cs \
	QueryParser.cs \
	Remote.cs \
	RSet.cs \
	SWIGTYPE_p_std__mapTstd__string_unsigned_int_t.cs \
	SWIGTYPE_p_std__string.cs \
	SWIGTYPE_p_std__vectorTstd__string_t.cs \
	SWIGTYPE_p_std__vectorTXapian__Query_t.cs \
	SWIGTYPE_p_std__vectorTXapian__StringAndFrequency_t.cs \
	SimpleStopper.cs \
	Stem.cs \
	Stopper.cs \
	StringValueRangeProcessor.cs \
	TermGenerator.cs \
	TermIterator.cs \
	TradWeight.cs \
	ValueIterator.cs \
	ValueRangeProcessor.cs \
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

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_CS_SRCS)

EXTRA_DIST = util.i AssemblyInfo.cs.in SmokeTest.cs $(BUILT_SOURCES)

lib_LTLIBRARIES = XapianSharp.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
XapianSharp_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
XapianSharp_la_SOURCES = xapian_wrap.cc
XapianSharp_la_LIBADD = $(XAPIAN_LIBS)

install-data-local: $(ASSEMBLY).dll
	$(GACUTIL) /i $(ASSEMBLY).dll /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

uninstall-local:
	$(GACUTIL) /u $(ASSEMBLY) /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

SmokeTest.exe: SmokeTest.cs $(ASSEMBLY).dll
	$(CSC) -unsafe -target:exe -out:SmokeTest.exe `test -f SmokeTest.cs||echo '$(srcdir)/'`SmokeTest.cs -r:$(ASSEMBLY).dll

all-am: $(ASSEMBLY).dll

CLEANFILES = XapianSharp.snk AssemblyInfo.cs $(ASSEMBLY).dll SmokeTest.exe

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_CS_SRCS): xapian_wrap.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf xapian_wrap.lock xapian_wrap.stamp' 1 2 13 15; \
	  if mkdir xapian_wrap.lock 2>/dev/null; then \
	    rm -f xapian_wrap.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) xapian_wrap.stamp; \
	    rmdir xapian_wrap.lock; \
	  else \
	    while test -d xapian_wrap.lock; do sleep 1; done; \
	    test -f xapian_wrap.stamp; exit $$?; \
	  fi; \
	fi
xapian_wrap.stamp: $(SWIG_sources) util.i
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f $(XAPIAN_SWIG_CS_SRCS)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -csharp -namespace Xapian -module Xapian -dllimport XapianSharp \
	    -o xapian_wrap.cc $(SWIG_mainsource)
	touch $@

CLEANFILES += $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
