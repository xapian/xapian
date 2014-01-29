## Process this file with automake to produce Makefile.in

.PHONY: check-perf

check-perf: perftest/perftest$(EXEEXT) perftest/get_machine_info
	VALGRIND= XAPIAN_TESTSUITE_LD_PRELOAD= $(TESTS_ENVIRONMENT) ./perftest/perftest$(EXEEXT)

## Programs to build
check_PROGRAMS += perftest/perftest

# Ensure the get_machine_info script is up to date before running tests.
check_SCRIPTS += perftest/get_machine_info
perftest/get_machine_info: perftest/get_machine_info.in
	cd .. && $(MAKE) tests/perftest/get_machine_info

## Sources:

noinst_HEADERS += perftest/perftest.h

collated_perftest_sources = \
 perftest/perftest_matchdecider.cc \
 perftest/perftest_randomidx.cc

perftest_perftest_SOURCES = perftest/perftest.cc $(collated_perftest_sources) \
 perftest/perftest_all.h perftest/perftest_collated.h \
 perftest/freemem.cc perftest/freemem.h \
 perftest/runprocess.cc perftest/runprocess.h \
 $(testharness_sources)
perftest_perftest_LDFLAGS = @NO_INSTALL@ $(ldflags)
perftest_perftest_LDADD = ../libgetopt.la ../$(libxapian_la)

if MAINTAINER_MODE
BUILT_SOURCES += perftest/perftest_all.h perftest/perftest_collated.h \
 $(collated_perftest_sources:.cc=.h)

perftest/perftest_all.h perftest/perftest_collated.h $(collated_perftest_sources:.cc=.h): perftest/perftest_collated.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf perftest/perftest_collated.lock perftest/perftest_collated.stamp' 1 2 13 15; \
	  if mkdir perftest/perftest_collated.lock 2>/dev/null; then \
	    rm -f perftest/perftest_collated.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) perftest/perftest_collated.stamp; \
	    rmdir perftest/perftest_collated.lock; \
	  else \
	    while test -d perftest/perftest_collated.lock; do sleep 1; done; \
	    test -f perftest/perftest_collated.stamp; exit $$?; \
	  fi; \
	fi
perftest/perftest_collated.stamp: $(collated_perftest_sources) collate-test perftest/Makefile.mk
	$(PERL) "$(srcdir)/collate-test" "$(srcdir)" perftest/perftest_collated.h perftest/perftest_all.h $(collated_perftest_sources)
	touch $@
endif

EXTRA_DIST += perftest/dir_contents \
	perftest/perftest_all.h perftest/perftest_collated.h \
	$(collated_perftest_sources:.cc=.h)
