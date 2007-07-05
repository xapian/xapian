## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

#SUBDIRS = docs

TESTS_ENVIRONMENT = \
  LIBTOOL="$(LIBTOOL)" JAVA="$(JAVA)" PATHSEP="$(JAVA_PATHSEP)" $(srcdir)/run-java-test

## Test programs to be run
TESTS = SmokeTest.class

CLEANFILES = SmokeTest.class built/xapian_jni.jar\
	$(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)\
	Query*1.class

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) -classpath $(srcdir)$(JAVA_PATHSEP). -d . $<

XAPIAN_SWIG_JAVA_SRCS=\
	Auto.java\
	BM25Weight.java\
	BoolWeight.java\
	Database.java\
	DateValueRangeProcessor.java\
	Document.java\
	Enquire.java\
	ESet.java\
	ESetIterator.java\
	ExpandDecider.java\
	Flint.java\
	InMemory.java\
	MatchDecider.java\
	MSet.java\
	MSetIterator.java\
	NumberValueRangeProcessor.java\
	PositionIterator.java\
	PostingIterator.java\
	Quartz.java\
	Query.java\
	QueryParser.java\
	Remote.java\
	RSet.java\
	SimpleStopper.java\
	Stem.java\
	Stopper.java\
	StringValueRangeProcessor.java\
	SWIGTYPE_p_std__string.java\
	TermGenerator.java\
	TermIterator.java\
	TradWeight.java\
	ValueIterator.java\
	ValueRangeProcessor.java\
	Version.java\
	Weight.java\
	WritableDatabase.java\
	Xapian.java\
	XapianConstants.java\
	XapianJNI.java

XAPIAN_SWIG_JAVA_CLASS = $(XAPIAN_SWIG_JAVA_SRCS:.java=.class)

# Java generates nested classes with filenames containing a $ (smart move) so
# we pick them up with a wildcard and omit them from dependencies to avoid
# escaping hell.  The lack of a dependency shouldn't really be an issue since
# these classes are always generated along with the containing class which
# is listed in the dependencies.
XAPIAN_SWIG_JAVA_EXTRA_CLASSES=\
	Enquire*docid_order.class\
	Query*op.class\
	QueryParser*feature_flag.class\
	QueryParser*stem_strategy.class

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar: $(XAPIAN_SWIG_JAVA_CLASS)
	-mkdir built
	$(JAR) -cf built/xapian_jni.jar $(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)

jnidir = `pwd`/built

jni_LTLIBRARIES = libxapian_jni.la

# Remove the .la file - libxapian_jni.la is never linked against (it's a
# module) and JVMs don't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(jnidir)/xapian.la

AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
AM_CPPFLAGS = $(JAVA_CPPFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
libxapian_jni_la_SOURCES = xapian_wrap.cc

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS)

EXTRA_DIST = run-java-test SmokeTest.java $(BUILT_SOURCES)

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS): xapian_wrap.stamp
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
	-rm -f $(XAPIAN_SWIG_JAVA_SRCS)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -java -module Xapian \
	    -o xapian_wrap.cc $(SWIG_mainsource)
	: # -package org.xapian
	: # Insert code to automatically load the JNI library.
	$(PERL) -pi -e 'print "    System.loadLibrary(\"xapian_jni\");\n" if /^\s*swig_module_init/' XapianJNI.java
	: # Check that all the sources we expected were generated.
	ls $(XAPIAN_SWIG_JAVA_SRCS) > /dev/null
	touch $@

CLEANFILES += $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
