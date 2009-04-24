if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

SUFFIXES = .java .class

EXTRA_DIST = AssertionError.java DatabaseCorruptError.java DatabaseError.java\
 DatabaseLockError.java DatabaseModifiedError.java DatabaseOpeningError.java\
 DocNotFoundError.java FeatureUnavailableError.java\
 InternalError.java InvalidArgumentError.java\
 InvalidOperationError.java LogicError.java\
 NetworkError.java NetworkTimeoutError.java RangeError.java RuntimeError.java\
 UnimplementedError.java XapianError.java XapianRuntimeError.java

XAPIAN_CLASS_FILES = AssertionError.class DatabaseCorruptError.class\
 DatabaseError.class DatabaseLockError.class DatabaseModifiedError.class\
 DatabaseOpeningError.class DocNotFoundError.class\
 FeatureUnavailableError.class InternalError.class InvalidArgumentError.class\
 InvalidOperationError.class\
 LogicError.class NetworkError.class NetworkTimeoutError.class RangeError.class\
 RuntimeError.class UnimplementedError.class XapianError.class\
 XapianRuntimeError.class

noinst_DATA = $(XAPIAN_CLASS_FILES)

.java.class:
	-rm -f ../../../built/xapian_jni.jar
	$(JAVAC) -classpath $(srcdir)/../../..$(JAVA_PATHSEP)../../.. -d ../../.. $<

CLEANFILES = $(XAPIAN_CLASS_FILES)
