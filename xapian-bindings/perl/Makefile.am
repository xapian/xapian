## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

# Disable parallel build for this make invocation; some tests depend
# on previous ones having set up their database for them.
.NOTPARALLEL:

TESTS_ENVIRONMENT = \
	CPPFLAGS="$(CPPFLAGS)" \
	CXX="$(CXX)" \
	CXXFLAGS="$(CXXFLAGS)" \
	XAPIAN_CONFIG="$(XAPIAN_CONFIG)" \
	prove -I. -I"$(srcdir)"

## Test programs to be run
TESTS = \
	t/01use.t \
	t/02pod.t \
	t/03podcoverage.t \
	t/04functions.t \
	t/10query.t \
	t/collapse.t \
	t/create.t \
	t/databasemodified.t \
	t/document.t \
	t/exception.t \
	t/index.t \
	t/parser.t \
	t/search.t \
	t/sorter.t \
	t/stem.t \
	t/symbol-test.t \
	t/termgenerator.t \
	t/thread.t \
	t/tied.t \
	t/valuerange.t \
	t/writabledatabase.t

clean-local:
	rm -rf testdb* auto

BUILT_SOURCES = Search/Xapian.pm xapian_wrap.cc \
	auto/Search/Xapian/Xapian$(PERL_SO)

auto/Search/Xapian/Xapian$(PERL_SO): Xapian.la
	@$(MKDIR_P) auto/Search/Xapian
	$(LIBTOOL) --config > libtoolconfig.tmp
## ksh requires a path on the sourced file.
	. ./libtoolconfig.tmp; cp "$$objdir/Xapian$(PERL_SO)" auto/Search/Xapian
	rm -f libtoolconfig.tmp

EXTRA_DIST = except.i $(TESTS) \
	generate-perl-exceptions \
	Search/Xapian.pm xapian_wrap.cc \
	t/symbol-test/SymbolTest.pm \
	t/symbol-test/typemap \
	t/symbol-test/SymbolTest.xs \
	t/symbol-test/Makefile.PL

exampledatadir = $(docdir)/perl/examples
dist_exampledata_DATA = \
	docs/examples/full-indexer.pl \
	docs/examples/full-searcher.pl \
	docs/examples/simpleexpand.pl \
	docs/examples/simpleindex.pl \
	docs/examples/simplematchdecider.pl \
	docs/examples/simplesearch.pl

docdatadir = $(docdir)/perl
dist_docdata_DATA = docs/index.html

perlarchdir = $(PERL_ARCH)/auto/Search/Xapian
perlarch_LTLIBRARIES = Xapian.la

# Remove the .la file - Xapian.la is never linked against (it's a module)
# and Perl doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(perlarchdir)/Xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	eval `grep '^dlname=' $(perlarch_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(perlarchdir)/"$$dlname"

perllibdir = $(PERL_LIB)
nobase_dist_perllib_DATA = \
	Search/Xapian.pm \
	Search/Xapian/AssertionError.pm \
	Search/Xapian/BM25Weight.pm \
	Search/Xapian/BoolWeight.pm \
	Search/Xapian/DatabaseCorruptError.pm \
	Search/Xapian/DatabaseCreateError.pm \
	Search/Xapian/DatabaseError.pm \
	Search/Xapian/DatabaseLockError.pm \
	Search/Xapian/DatabaseModifiedError.pm \
	Search/Xapian/DatabaseOpeningError.pm \
	Search/Xapian/Database.pm \
	Search/Xapian/DatabaseVersionError.pm \
	Search/Xapian/DocNotFoundError.pm \
	Search/Xapian/Document.pm \
	Search/Xapian/Enquire.pm \
	Search/Xapian/Error.pm \
	Search/Xapian/ESetIterator.pm \
	Search/Xapian/ESet.pm \
	Search/Xapian/FeatureUnavailableError.pm \
	Search/Xapian/InternalError.pm \
	Search/Xapian/InvalidArgumentError.pm \
	Search/Xapian/InvalidOperationError.pm \
	Search/Xapian/LogicError.pm \
	Search/Xapian/MSetIterator.pm \
	Search/Xapian/MSet.pm \
	Search/Xapian/MultiValueSorter.pm \
	Search/Xapian/NetworkError.pm \
	Search/Xapian/NetworkTimeoutError.pm \
	Search/Xapian/PerlStopper.pm \
	Search/Xapian/PositionIterator.pm \
	Search/Xapian/PostingIterator.pm \
	Search/Xapian/QueryParserError.pm \
	Search/Xapian/QueryParser.pm \
	Search/Xapian/Query.pm \
	Search/Xapian/RangeError.pm \
	Search/Xapian/RSet.pm \
	Search/Xapian/RuntimeError.pm \
	Search/Xapian/SimpleStopper.pm \
	Search/Xapian/Stem.pm \
	Search/Xapian/Stopper.pm \
	Search/Xapian/TermGenerator.pm \
	Search/Xapian/TermIterator.pm \
	Search/Xapian/TradWeight.pm \
	Search/Xapian/UnimplementedError.pm \
	Search/Xapian/ValueIterator.pm \
	Search/Xapian/Weight.pm \
	Search/Xapian/WritableDatabase.pm

AM_CPPFLAGS = -I$(PERL_INC)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
Xapian_la_LDFLAGS = -avoid-version -module -shrext $(PERL_SO) $(NO_UNDEFINED)
Xapian_la_SOURCES = xapian_wrap.cc
Xapian_la_LIBADD = $(XAPIAN_LIBS) $(PERL_LIBS)

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-perl-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-perl-exceptions

BUILT_SOURCES += docs/index.html
docs/index.html: Search/Xapian.pm
	test -d docs || mkdir docs
	pod2html Search/Xapian.pm > docs/index.html
	rm -f pod2htm?.tmp

xapian_wrap.cc Search/Xapian.pm: xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i except.i
	$(multitarget_begin)
	test -d Search || mkdir Search
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -perl -module Search::Xapian -proxy -const \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	mv Xapian.pm Search/Xapian.pm
	mv xapian_wrap_d.tmp xapian_wrap.d
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES = pod2htm?.tmp xapian_wrap.d xapian_wrap.stamp
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
