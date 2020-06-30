## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.12.2 serial-tests subdir-objects

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/include \
 -I$(top_builddir)/include -I$(srcdir)/harness
AM_CXXFLAGS += $(XAPIAN_CXXFLAGS)

CLEANFILES =

TESTS_ENVIRONMENT = ./runtest

.PHONY: up remove-cached-databases

up:
	cd .. && $(MAKE)

## Test programs to be run
TESTS = apitest$(EXEEXT)

## Programs to build
check_PROGRAMS = \
	apitest

# Make sure runtest is up to date before running tests
check_SCRIPTS = runtest

## Sources:

noinst_HEADERS = apitest.h

collated_apitest_sources = \
 api_generated.cc \
 api_letor.cc

apitest_SOURCES = apitest.cc $(collated_apitest_sources) \
 api_all.h api_collated.h $(testharness_sources)

apitest_LDFLAGS = $(NO_INSTALL) $(XAPIAN_LDFLAGS)
apitest_LDADD = ../libgetopt.la $(XAPIAN_LIBS) ../libxapianletor.la

BUILT_SOURCES =

if MAINTAINER_MODE
BUILT_SOURCES += api_all.h api_collated.h $(collated_apitest_sources:.cc=.h) \
 api_collated.stamp api_generated.cc

api_all.h api_collated.h $(collated_apitest_sources:.cc=.h): api_collated.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf api_collated.lock api_collated.stamp' 1 2 13 15; \
	  if mkdir api_collated.lock 2>/dev/null; then \
	    rm -f api_collated.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) api_collated.stamp; \
	    rmdir api_collated.lock; \
	  else \
	    while test -d api_collated.lock; do sleep 1; done; \
	    test -f api_collated.stamp; exit $$?; \
	  fi; \
	fi
api_collated.stamp: $(collated_apitest_sources) collate-test Makefile.am
	$(PERL) "$(srcdir)/collate-test" "$(srcdir)" api_collated.h api_all.h $(collated_apitest_sources)
	touch $@

api_generated.cc: generate-api_generated
	$(PERL) "$(srcdir)/generate-api_generated" > api_generated.ccT
	mv api_generated.ccT api_generated.cc
endif

EXTRA_DIST = collate-test generate-api_generated api_all.h api_collated.h \
	$(collated_apitest_sources:.cc=.h) api_collated.stamp

## Distribute test data:
EXTRA_DIST +=\
	runtest.in \
	valgrind.supp \
	testdata/blank_space_before_query.txt \
	testdata/empty_file.txt \
	testdata/nosinglequotes.txt \
	testdata/nospace.txt \
	testdata/parse_query_empty_string.txt \
	testdata/parse_query_noclosingquote.txt \
	testdata/parse_query_noopenquote.txt \
	testdata/parse_query_valid.txt \
	testdata/qrel.txt \
	testdata/qrelone.txt \
	testdata/qrelthree_correct.txt \
	testdata/query.txt \
	testdata/queryone.txt \
	testdata/querythree.txt \
	testdata/score_qrel.txt \
	testdata/training_data.txt \
	testdata/training_data_ndcg.txt \
	testdata/training_data_one_document.txt \
	testdata/training_data_three_correct.txt \
	testdata/unique_query_id.txt

CLEANFILES +=\
	err_output_listmle_1.txt \
	err_output_listmle_2.txt \
	err_output_listmle_3.txt \
	err_output_listnet_1.txt \
	err_output_listnet_2.txt \
	err_output_listnet_3.txt \
	err_output_svm_1.txt \
	err_output_svm_2.txt \
	err_output_svm_3.txt \
	ndcg_output_listmle_1.txt \
	ndcg_output_listmle_2.txt \
	ndcg_output_listmle_3.txt \
	ndcg_output_listnet_1.txt \
	ndcg_output_listnet_2.txt \
	ndcg_output_listnet_3.txt \
	ndcg_output_svm_1.txt \
	ndcg_output_svm_2.txt \
	ndcg_output_svm_3.txt \
	ndcg_score_output.txt \
	ndcg_score_test.txt \
	training_output_data_one_doc.txt \
	training_output_empty.txt \
	training_output_three_correct.txt \
	training_output.txt \
	training_output1.txt

remove-cached-databases:
	rm -rf .glass .honey .multiglass .replicatmp .singlefileglass .stub

clean-local: remove-cached-databases

# Remove the cached databases before "make check" to avoid issues with missing
# bugs in database creation when we reuse databases from a previous test run.
check: remove-cached-databases

include harness/Makefile.mk

EXTRA_DIST += zlib-vg.c
if USE_ZLIB_VG
check_SCRIPTS += zlib-vg.so
CLEANFILES += zlib-vg.so

zlib-vg.so: zlib-vg.c
	$(CC) -fPIC -shared -o zlib-vg.so $(srcdir)/zlib-vg.c -ldl
endif

.PHONY: check-syntax

MAINTAINERCLEANFILES = $(BUILT_SOURCES)

check-syntax:
	$(CXXCOMPILE) -fsyntax-only -gstabs+ $(CHK_SOURCES)
