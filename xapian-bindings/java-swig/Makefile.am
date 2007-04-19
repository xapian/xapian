## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

#SUBDIRS = docs

TESTS_ENVIRONMENT = \
  LIBTOOL="$(LIBTOOL)" JAVA="$(JAVA)" PATHSEP="$(JAVA_PATHSEP)" $(srcdir)/run-java-test

## Test programs to be run
TESTS = SmokeTest.class

EXTRA_DIST = run-java-test SmokeTest.java

CLEANFILES = SmokeTest.class $(XAPIAN_SWIG_JAVA_CLASS)
	built/xapian_jni.jar

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) -classpath $(srcdir)$(JAVA_PATHSEP). -d . $<

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar: $(XAPIAN_SWIG_JAVA_CLASS)
	-mkdir built
	$(JAR) -cf built/xapian_jni.jar $(XAPIAN_SWIG_JAVA_CLASS)

jnidir = `pwd`/built

jni_LTLIBRARIES = libxapian_jni.la

XAPIAN_SWIG_JAVA_SRCS=\
	Auto.java\
	BoolWeight.java\
	BM25Weight.java\
	Database.java\
	Document.java\
	Enquire.java\
	ExpandDecider.java\
	ESet.java\
	ESetIterator.java\
	Flint.java\
	InMemory.java\
	MatchDecider.java\
	MSet.java\
	MSetIterator.java\
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
	TermIterator.java\
	TradWeight.java\
	ValueIterator.java\
	Version.java\
	Weight.java\
	WritableDatabase.java\
	Xapian.java\
	XapianConstants.java\
	XapianJNI.java


XAPIAN_SWIG_JAVA_CLASS=\
	$(XAPIAN_SWIG_JAVA_SRCS:.java=.class)\
	Enquire*docid_order.class\
	Query*1.class\
	Query*op.class\
	QueryParser*feature_flag.class\
	QueryParser*stem_strategy.class

# Remove the .la file - libxapian_jni.la is never linked against (it's a
# module) and JVMs don't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(jnidir)/xapian.la

AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
libxapian_jni_la_SOURCES = xapian_wrap.cc

BUILT_SOURCES = xapian_wrap.cc $(XAPIAN_SWIG_JAVA_SRCS)

if MAINTAINER_MODE
xapian_wrap.cc $(XAPIAN_SWIG_JAVA_SRCS): $(SWIG_sources) util.i
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f $(XAPIAN_SWIG_JAVA_SRCS)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -java -module Xapian \
	    -o xapian_wrap.cc $(SWIG_mainsource)
	: # -package org.xapian
	: # Check that all the sources we expected were generated.
	ls $(XAPIAN_SWIG_JAVA_SRCS) > /dev/null

CLEANFILES += $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
