## Process this file with automake to produce Makefile.in

.PHONY: check-soak

check-soak: soaktest/soaktest$(EXEEXT)
	$(TESTS_ENVIRONMENT) ./soaktest/soaktest$(EXEEXT)

## Programs to build
check_PROGRAMS += soaktest/soaktest

## Sources:
noinst_HEADERS += soaktest/soaktest.h

collated_soaktest_sources = \
 soaktest/soaktest_queries.cc

soaktest_soaktest_SOURCES = \
 soaktest/soaktest.cc \
 $(collated_soaktest_sources) \
 soaktest/soaktest_all.h \
 soaktest/soaktest_collated.h \
 $(testharness_sources)
soaktest_soaktest_LDFLAGS = $(NO_INSTALL) $(ldflags)
soaktest_soaktest_LDADD = ../libgetopt.la ../$(libxapian_la)

if MAINTAINER_MODE
BUILT_SOURCES += soaktest/soaktest_all.h soaktest/soaktest_collated.h \
 $(collated_soaktest_sources:.cc=.h)

soaktest/soaktest_all.h soaktest/soaktest_collated.h $(collated_soaktest_sources:.cc=.h): soaktest/soaktest_collated.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf soaktest/soaktest_collated.lock soaktest/soaktest_collated.stamp' 1 2 13 15; \
	  if mkdir soaktest/soaktest_collated.lock 2>/dev/null; then \
	    rm -f soaktest/soaktest_collated.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) soaktest/soaktest_collated.stamp; \
	    rmdir soaktest/soaktest_collated.lock; \
	  else \
	    while test -d soaktest/soaktest_collated.lock; do sleep 1; done; \
	    test -f soaktest/soaktest_collated.stamp; exit $$?; \
	  fi; \
	fi
soaktest/soaktest_collated.stamp: $(collated_soaktest_sources) collate-test soaktest/Makefile.mk
	$(PERL) "$(srcdir)/collate-test" "$(srcdir)" soaktest/soaktest_collated.h soaktest/soaktest_all.h $(collated_soaktest_sources)
	touch $@
endif

EXTRA_DIST += \
 soaktest/dir_contents \
 soaktest/soaktest_all.h \
 soaktest/soaktest_collated.h \
 $(collated_soaktest_sources:.cc=.h)
