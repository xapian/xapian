## Process this file with automake to produce Makefile.in

SUBDIRS = native org .

TESTS_ENVIRONMENT = \
  LIBTOOL="$(LIBTOOL)" JAVA="$(JAVA)" PATHSEP="$(JAVA_PATHSEP)" ./run-java-test

## Test programs to be run
TESTS = SmokeTest

EXTRA_DIST = run-java-test SmokeTest.java

CLEANFILES = SmokeTest.class built/xapian_jni.jar

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) -classpath $(srcdir)$(JAVA_PATHSEP). -d . $<

# directory to install xapian_jni.jar in
# `pwd`/
jardir = built

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar:
	$(JAR) -cf built/xapian_jni.jar org/xapian/*.class org/xapian/errors/*.class
