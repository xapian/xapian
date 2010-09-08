## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

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
	Brass.java\
	Chert.java\
	Database.java\
	DateValueRangeProcessor.java\
	DecreasingValueWeightPostingSource.java\
	Document.java\
	Enquire.java\
	ESet.java\
	ESetIterator.java\
	ExpandDecider.java\
	FixedWeightPostingSource.java\
	Flint.java\
	InMemory.java\
	KeyMaker.java\
	MatchDecider.java\
	MatchSpy.java\
	MSet.java\
	MSetIterator.java\
	MultiValueKeyMaker.java\
	MultiValueSorter.java\
	NumberValueRangeProcessor.java\
	PositionIterator.java\
	PostingIterator.java\
	PostingSource.java\
	Query.java\
	QueryParser.java\
	Registry.java\
	Remote.java\
	RSet.java\
	SimpleStopper.java\
	Sorter.java\
	Stem.java\
	StemImplementation.java\
	Stopper.java\
	StringValueRangeProcessor.java\
	SWIGTYPE_p_std__string.java\
	TermGenerator.java\
	TermIterator.java\
	TradWeight.java\
	ValueCountMatchSpy.java\
	ValueIterator.java\
	ValueMapPostingSource.java\
	ValuePostingSource.java\
	ValueRangeProcessor.java\
	ValueSetMatchDecider.java\
	ValueWeightPostingSource.java\
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
	QueryParser*stem_strategy.class\
	TermGenerator*flags.class

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar: $(XAPIAN_SWIG_JAVA_CLASS)
	-test -d built || mkdir built
	$(JAR) -cf built/xapian_jni.jar $(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)

jnidir = `pwd`/built

jni_LTLIBRARIES = libxapian_jni.la

# Remove the .la file - libxapian_jni.la is never linked against (it's a
# module) and JVMs don't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(jnidir)/xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	eval `grep '^dlname=' $(jni_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(jnidir)/"$$dlname"

AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
AM_CPPFLAGS = $(JAVA_CPPFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
libxapian_jni_la_SOURCES = xapian_wrap.cc

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS)

EXTRA_DIST = run-java-test SmokeTest.java $(BUILT_SOURCES)

if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS): xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)						       
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f $(XAPIAN_SWIG_JAVA_SRCS)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -java -module Xapian \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pe 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp > xapian_wrap_d2.tmp
	mv xapian_wrap_d2.tmp xapian_wrap.d
	rm xapian_wrap_d.tmp
	: # -package org.xapian
	: # Insert code to automatically load the JNI library.
	$(PERL) -pi -e 'print "    System.loadLibrary(\"xapian_jni\");\n" if /^\s*swig_module_init/' XapianJNI.java
	: # Check that all the sources we expected were generated.
	ls $(XAPIAN_SWIG_JAVA_SRCS) > /dev/null
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
