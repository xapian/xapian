## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = $(RUN_CSHARP)

## Test programs to be run
TESTS = SmokeTest.exe

ASSEMBLY=XapianSharp

XAPIAN_SWIG_CS_SRCS=\
	generated-csharp/Auto.cs \
	generated-csharp/BM25Weight.cs \
	generated-csharp/BoolWeight.cs \
	generated-csharp/Brass.cs \
	generated-csharp/Chert.cs \
	generated-csharp/Compactor.cs \
	generated-csharp/Database.cs \
	generated-csharp/DateValueRangeProcessor.cs \
	generated-csharp/DecreasingValueWeightPostingSource.cs \
	generated-csharp/Document.cs \
	generated-csharp/ESet.cs \
	generated-csharp/ESetIterator.cs \
	generated-csharp/Enquire.cs \
	generated-csharp/ExpandDecider.cs \
	generated-csharp/FixedWeightPostingSource.cs \
	generated-csharp/Flint.cs \
	generated-csharp/InMemory.cs \
	generated-csharp/KeyMaker.cs \
	generated-csharp/MatchDecider.cs \
	generated-csharp/MatchSpy.cs \
	generated-csharp/MSet.cs \
	generated-csharp/MSetIterator.cs \
	generated-csharp/MultiValueKeyMaker.cs \
	generated-csharp/MultiValueSorter.cs \
	generated-csharp/NumberValueRangeProcessor.cs \
	generated-csharp/PositionIterator.cs \
	generated-csharp/PostingIterator.cs \
	generated-csharp/PostingSource.cs \
	generated-csharp/Query.cs \
	generated-csharp/QueryParser.cs \
	generated-csharp/Remote.cs \
	generated-csharp/RSet.cs \
	generated-csharp/SWIGTYPE_p_std__string.cs \
	generated-csharp/SWIGTYPE_p_std__vectorT_std__string_t.cs \
	generated-csharp/SWIGTYPE_p_std__vectorT_Xapian__Query_t.cs \
	generated-csharp/Registry.cs \
	generated-csharp/SimpleStopper.cs \
	generated-csharp/Sorter.cs \
	generated-csharp/Stem.cs \
	generated-csharp/StemImplementation.cs \
	generated-csharp/Stopper.cs \
	generated-csharp/StringValueRangeProcessor.cs \
	generated-csharp/TermGenerator.cs \
	generated-csharp/TermIterator.cs \
	generated-csharp/TradWeight.cs \
	generated-csharp/ValueCountMatchSpy.cs \
	generated-csharp/ValueIterator.cs \
	generated-csharp/ValueMapPostingSource.cs \
	generated-csharp/ValuePostingSource.cs \
	generated-csharp/ValueRangeProcessor.cs \
	generated-csharp/ValueSetMatchDecider.cs \
	generated-csharp/ValueWeightPostingSource.cs \
	generated-csharp/Version.cs \
	generated-csharp/Weight.cs \
	generated-csharp/WritableDatabase.cs \
	generated-csharp/Xapian.cs \
	generated-csharp/XapianPINVOKE.cs

XapianSharp.snk:
	$(SN) -k $@

$(ASSEMBLY).dll: $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs XapianSharp.snk
## We specify both -keyfile and -keycontainer - hopefully this means that if
## this is the first time we're signing, the key in the file is used and
## then stored in the key container.  From then on, the key in the container
## is used.
	$(CSC) -unsafe -target:library -out:$(ASSEMBLY).dll \
	    -keyfile:XapianSharp.snk -keycontainer:"Xapian Signing Key" \
	    `for f in $(XAPIAN_SWIG_CS_SRCS); do if test -f "$$f"; then echo $$f; else echo $(srcdir)/$$f ; fi ; done` \
	    AssemblyInfo.cs

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_CS_SRCS)

EXTRA_DIST = util.i SmokeTest.cs $(BUILT_SOURCES)

lib_LTLIBRARIES = _XapianSharp.la

# Remove the .la file - _XapianSharp.la is never linked against (it's a module)
# and C# doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(libdir)/_XapianSharp.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	eval `grep '^dlname=' $(lib_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(libdir)/"$$dlname"
	$(GACUTIL) /u $(ASSEMBLY) /f $(GACUTIL_FLAGS)

AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_XapianSharp_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
_XapianSharp_la_SOURCES = xapian_wrap.cc
_XapianSharp_la_LIBADD = $(XAPIAN_LIBS)

install-data-local: $(ASSEMBLY).dll
	$(GACUTIL) /i $(ASSEMBLY).dll /f $(GACUTIL_FLAGS)

SmokeTest.exe: SmokeTest.cs $(ASSEMBLY).dll
	$(CSC) -unsafe -target:exe -out:SmokeTest.exe `test -f SmokeTest.cs||echo '$(srcdir)/'`SmokeTest.cs -r:$(ASSEMBLY).dll

all-am: $(ASSEMBLY).dll

CLEANFILES = XapianSharp.snk $(ASSEMBLY).dll SmokeTest.exe

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_CS_SRCS): xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f generated-csharp/*
	$(MKDIR_P) generated-csharp
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -csharp -namespace Xapian -module Xapian -dllimport _XapianSharp \
	    -o xapian_wrap.cc -outdir generated-csharp \
	    -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pi -e 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp
	mv xapian_wrap_d.tmp xapian_wrap.d
	: # Check that exactly the sources we expected were generated.
	$(PERL) -e 'print "$$_\n" for sort @ARGV' generated-csharp/*.cs > generated-csharp/list
	$(PERL) -e 'print "$$_\n" for sort @ARGV' $(XAPIAN_SWIG_CS_SRCS)|diff -c - generated-csharp/list
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += xapian_wrap.stamp
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

maintainer-clean-local:
	rm -rf generated-csharp

exampledatadir = $(docdir)/csharp/examples
dist_exampledata_DATA =\
	docs/examples/SimpleSearch.cs\
	docs/examples/SimpleIndex.cs\
	docs/examples/SimpleExpand.cs

docdatadir = $(docdir)/csharp
dist_docdata_DATA = docs/index.html
