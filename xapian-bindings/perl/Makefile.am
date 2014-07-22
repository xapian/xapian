## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

# Disable parallel build for this make invocation; some tests depend
# on previous ones having set up their database for them.
.NOTPARALLEL:

## Test programs to be run
TESTS = \
	t/01use.t \
	t/02pod.t \
	t/03podcoverage.t \
	t/04functions.t \
	t/10query.t \
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
AM_TESTS_ENVIRONMENT = \
	CPPFLAGS="$(CPPFLAGS)" ;\
	CXX="$(CXX)" ;\
	CXXFLAGS="$(CXXFLAGS)" ;\
	XAPIAN_CONFIG="$(XAPIAN_CONFIG)" ;\
	export CPPFLAGS ;\
	export CXX ;\
	export CXXFLAGS ;\
	export XAPIAN_CONFIG ;
LOG_COMPILER = $(PROVE)
AM_LOG_FLAGS = -I. -I'$(srcdir)'

clean-local:
	rm -rf testdb* auto

BUILT_SOURCES = Xapian.pm xapian_wrap.cc \
	auto/Xapian/Xapian$(PERL_SO)

auto/Xapian/Xapian$(PERL_SO): Xapian.la
	@$(MKDIR_P) auto/Xapian
	$(LIBTOOL) --config > libtoolconfig.tmp
## ksh requires a path on the sourced file.
	. ./libtoolconfig.tmp; cp "$$objdir/Xapian$(PERL_SO)" auto/Xapian
	rm -f libtoolconfig.tmp

EXTRA_DIST = perl.i except.i $(TESTS) \
	generate-perl-exceptions \
	Xapian.pm xapian_wrap.cc \
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

perlarchdir = $(PERL_ARCH)/auto/Xapian
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
	Xapian.pm \
	Xapian/AssertionError.pm \
	Xapian/BM25Weight.pm \
	Xapian/BoolWeight.pm \
	Xapian/DatabaseCorruptError.pm \
	Xapian/DatabaseCreateError.pm \
	Xapian/DatabaseError.pm \
	Xapian/DatabaseLockError.pm \
	Xapian/DatabaseModifiedError.pm \
	Xapian/DatabaseOpeningError.pm \
	Xapian/Database.pm \
	Xapian/DatabaseVersionError.pm \
	Xapian/DocNotFoundError.pm \
	Xapian/Document.pm \
	Xapian/Enquire.pm \
	Xapian/Error.pm \
	Xapian/ESetIterator.pm \
	Xapian/ESet.pm \
	Xapian/FeatureUnavailableError.pm \
	Xapian/InternalError.pm \
	Xapian/InvalidArgumentError.pm \
	Xapian/InvalidOperationError.pm \
	Xapian/LogicError.pm \
	Xapian/MSetIterator.pm \
	Xapian/MSet.pm \
	Xapian/MultiValueSorter.pm \
	Xapian/NetworkError.pm \
	Xapian/NetworkTimeoutError.pm \
	Xapian/PerlStopper.pm \
	Xapian/PositionIterator.pm \
	Xapian/PostingIterator.pm \
	Xapian/QueryParserError.pm \
	Xapian/QueryParser.pm \
	Xapian/Query.pm \
	Xapian/RangeError.pm \
	Xapian/RSet.pm \
	Xapian/RuntimeError.pm \
	Xapian/SimpleStopper.pm \
	Xapian/Stem.pm \
	Xapian/Stopper.pm \
	Xapian/TermGenerator.pm \
	Xapian/TermIterator.pm \
	Xapian/TradWeight.pm \
	Xapian/UnimplementedError.pm \
	Xapian/ValueIterator.pm \
	Xapian/Weight.pm \
	Xapian/WritableDatabase.pm

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
docs/index.html: Xapian.pm
	$(MKDIR_P) docs
	pod2html Xapian.pm > docs/index.html
	rm -f pod2htm?.tmp

stamp = xapian_wrap.stamp
RUN_SWIG = stamp='$(stamp)' $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

xapian_wrap.cc Xapian.pm: $(stamp)
	$(make_many_locked)
$(stamp):
	$(multitarget_begin)
	$(RUN_SWIG) $(SWIG_WERROR) -I. -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -perl -module Xapian -proxy -const \
	    -o xapian_wrap.cc '$(srcdir)/'perl.i
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES = pod2htm?.tmp xapian_wrap.d $(stamp)
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
