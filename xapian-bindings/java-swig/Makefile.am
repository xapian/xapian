## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = \
  LIBTOOL="$(LIBTOOL)" JAVA="$(JAVA)" PATHSEP="$(JAVA_PATHSEP)" $(srcdir)/run-java-test

## Test programs to be run
TESTS = SmokeTest.class

CLEANFILES = SmokeTest.class MyMatchDecider.class MyExpandDecider.class\
	built/xapian_jni.jar\
	$(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)\
	Query*1.class

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) -classpath $(srcdir)$(JAVA_PATHSEP). -d . $<

XAPIAN_SWIG_JAVA_SRCS=\
	org/xapian/Auto.java\
	org/xapian/BM25Weight.java\
	org/xapian/BoolWeight.java\
	org/xapian/Brass.java\
	org/xapian/Chert.java\
	org/xapian/Compactor.java\
	org/xapian/Database.java\
	org/xapian/DateValueRangeProcessor.java\
	org/xapian/DecreasingValueWeightPostingSource.java\
	org/xapian/Document.java\
	org/xapian/Enquire.java\
	org/xapian/ESet.java\
	org/xapian/ESetIterator.java\
	org/xapian/ExpandDecider.java\
	org/xapian/FixedWeightPostingSource.java\
	org/xapian/Flint.java\
	org/xapian/InMemory.java\
	org/xapian/KeyMaker.java\
	org/xapian/MatchDecider.java\
	org/xapian/MatchSpy.java\
	org/xapian/MSet.java\
	org/xapian/MSetIterator.java\
	org/xapian/MultiValueKeyMaker.java\
	org/xapian/MultiValueSorter.java\
	org/xapian/NumberValueRangeProcessor.java\
	org/xapian/PositionIterator.java\
	org/xapian/PostingIterator.java\
	org/xapian/PostingSource.java\
	org/xapian/Query.java\
	org/xapian/QueryParser.java\
	org/xapian/Registry.java\
	org/xapian/Remote.java\
	org/xapian/RSet.java\
	org/xapian/SimpleStopper.java\
	org/xapian/Sorter.java\
	org/xapian/Stem.java\
	org/xapian/StemImplementation.java\
	org/xapian/Stopper.java\
	org/xapian/StringValueRangeProcessor.java\
	org/xapian/SWIGTYPE_p_std__string.java\
	org/xapian/TermGenerator.java\
	org/xapian/TermIterator.java\
	org/xapian/TradWeight.java\
	org/xapian/ValueCountMatchSpy.java\
	org/xapian/ValueIterator.java\
	org/xapian/ValueMapPostingSource.java\
	org/xapian/ValuePostingSource.java\
	org/xapian/ValueRangeProcessor.java\
	org/xapian/ValueSetMatchDecider.java\
	org/xapian/ValueWeightPostingSource.java\
	org/xapian/Version.java\
	org/xapian/Weight.java\
	org/xapian/WritableDatabase.java\
	org/xapian/Xapian.java\
	org/xapian/XapianConstants.java\
	org/xapian/XapianJNI.java

XAPIAN_SWIG_JAVA_CLASS = $(XAPIAN_SWIG_JAVA_SRCS:.java=.class)

# Java generates nested classes with filenames containing a $ (smart move) so
# we pick them up with a wildcard and omit them from dependencies to avoid
# escaping hell.  The lack of a dependency shouldn't really be an issue since
# these classes are always generated along with the containing class which
# is listed in the dependencies.
XAPIAN_SWIG_JAVA_EXTRA_CLASSES=\
	org/xapian/Compactor*compaction_level.class\
	org/xapian/Enquire*docid_order.class\
	org/xapian/Query*op.class\
	org/xapian/QueryParser*feature_flag.class\
	org/xapian/QueryParser*stem_strategy.class\
	org/xapian/TermGenerator*flags.class

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar: $(XAPIAN_SWIG_JAVA_CLASS)
	-test -d built || mkdir built
	$(JAR) -cf built/xapian_jni.jar org/xapian/*.class
##$(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)

jnidir = `pwd`/built

jni_LTLIBRARIES = libxapian_jni.la

# Remove the .la file - libxapian_jni.la is never linked against (it's a
# module) and JVMs don't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(jnidir)/libxapian_jni.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
if NEED_JNILIB_EXT
	rm -f $(DESTDIR)$(jnidir)/libxapian_jni.jnilib
else
	eval `grep '^dlname=' $(jni_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(jnidir)/"$$dlname"
endif

if NEED_JNILIB_EXT
SHREXT=-shrext .jnilib
else
SHREXT=
endif
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
AM_CPPFLAGS = $(JAVA_CPPFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(SHREXT) $(NO_UNDEFINED)
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
	-rm -f org/xapian/*
	$(MKDIR_P) org/xapian
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -java -module Xapian -package org.xapian -outdir org/xapian \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pi -e 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp
	mv xapian_wrap_d.tmp xapian_wrap.d
	: # Insert code to automatically load the JNI library.
	$(PERL) -pi -e 'print "    System.loadLibrary(\"xapian_jni\");\n" if /^\s*swig_module_init/' org/xapian/XapianJNI.java
	: # Check that exactly the sources we expected were generated.
	$(PERL) -e 'print "$$_\n" for sort @ARGV' org/xapian/*.java > org/xapian/list
	$(PERL) -e 'print "$$_\n" for sort @ARGV' $(XAPIAN_SWIG_JAVA_SRCS)|diff -c - org/xapian/list
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += xapian_wrap.stamp
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

maintainer-clean-local:
	rm -rf built org

exampledatadir = $(docdir)/java/examples
dist_exampledata_DATA =\
	docs/examples/SimpleSearch.java\
	docs/examples/SimpleIndex.java

docdatadir = $(docdir)/java
dist_docdata_DATA = docs/index.html
