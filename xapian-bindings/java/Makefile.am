## Process this file with automake to produce Makefile.in

SUBDIRS = native org .

## Test programs to be run

TESTS_ENVIRONMENT = JAVA=$(JAVA) ./run-java-test

## Test programs to be run
TESTS = SmokeTest

EXTRA_DIST = run-java-test SmokeTest.java

CLEANFILES = SmokeTest.class built/xapian_jni.jar

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) -classpath $(srcdir):. -d . $<

# directory to install xapian_jni.jar in
# `pwd`/
jardir = built

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar:
	$(JAR) -cf built/xapian_jni.jar org/xapian/*.class org/xapian/errors/*.class
