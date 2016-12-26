## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

# Disable parallel build for this make invocation; javac seems to write
# .class files in place, and it also reads existing .class files - the
# combination means that in a parallel build it can read partial .class
# and then fail with various odd errors.
.NOTPARALLEL:

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

SUBDIRS = native org/xapian .

TESTS_ENVIRONMENT = \
  LIBTOOL="$(LIBTOOL)" JAVA="$(JAVA)" PATHSEP="$(JAVA_PATHSEP)" $(srcdir)/run-java-test

## Test programs to be run
TESTS = SmokeTest.class

EXTRA_DIST = run-java-test SmokeTest.java

CLEANFILES = SmokeTest.class MyMatchDecider.class MyExpandDecider.class \
	built/xapian_jni.jar

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) -classpath $(srcdir)$(JAVA_PATHSEP). -d . $<

noinst_DATA = built/xapian_jni.jar

built/xapian_jni.jar:
	-test -d built || mkdir built
	$(JAR) -cf built/xapian_jni.jar org/xapian/*.class org/xapian/errors/*.class
