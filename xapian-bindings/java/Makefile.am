SUBDIRS = native org .

## Test programs to be run
## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT = JAVA=$(JAVA) ./run-java-test

## Test programs to be run
TESTS = SmokeTest

EXTRA_DIST = run-java-test

SmokeTest: SmokeTest.class

.java.class:
	$(JAVAC) $<
