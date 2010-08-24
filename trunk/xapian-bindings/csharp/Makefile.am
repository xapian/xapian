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
	Brass.cs \
	Chert.cs \
	Database.cs \
	DateValueRangeProcessor.cs \
	Document.cs \
	ESet.cs \
	ESetIterator.cs \
	Enquire.cs \
	ExpandDecider.cs \
	Flint.cs \
	InMemory.cs \
	KeyMaker.cs \
	MatchDecider.cs \
	MatchSpy.cs \
	MSet.cs \
	MSetIterator.cs \
	MultiValueKeyMaker.cs \
	MultiValueSorter.cs \
	NumberValueRangeProcessor.cs \
	PositionIterator.cs \
	PostingIterator.cs \
	PostingSource.cs \
	Query.cs \
	QueryParser.cs \
	Remote.cs \
	RSet.cs \
	SWIGTYPE_p_std__string.cs \
	SWIGTYPE_p_std__vectorT_std__string_t.cs \
	SWIGTYPE_p_std__vectorT_Xapian__Query_t.cs \
	Registry.cs \
	SimpleStopper.cs \
	Sorter.cs \
	Stem.cs \
	StemImplementation.cs \
	Stopper.cs \
	StringValueRangeProcessor.cs \
	TermGenerator.cs \
	TermIterator.cs \
	TradWeight.cs \
	ValueCountMatchSpy.cs \
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

lib_LTLIBRARIES = _XapianSharp.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_XapianSharp_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
_XapianSharp_la_SOURCES = xapian_wrap.cc
_XapianSharp_la_LIBADD = $(XAPIAN_LIBS)

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
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f $(XAPIAN_SWIG_CS_SRCS)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -csharp -namespace Xapian -module Xapian -dllimport _XapianSharp \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pe 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp > xapian_wrap_d2.tmp
	mv xapian_wrap_d2.tmp xapian_wrap.d
	rm xapian_wrap_d.tmp
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += $(BUILT_SOURCES) xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
