SUFFIXES = .java .class

EXTRA_DIST = AssertionError.java DataFlowError.java DatabaseCorruptError.java\
 DatabaseError.java DatabaseLockError.java DatabaseModifiedError.java\
 DatabaseOpeningError.java DocNotFoundError.java FeatureUnavailableError.java\
 InternalError.java InvalidArgumentError.java InvalidDataError.java\
 InvalidOperationError.java InvalidResultError.java LogicError.java\
 NetworkError.java NetworkTimeoutError.java RangeError.java RuntimeError.java\
 TypeError.java UnimplementedError.java XapianError.java

XAPIAN_CLASS_FILES = AssertionError.class DataFlowError.class\
 DatabaseCorruptError.class DatabaseError.class DatabaseLockError.class\
 DatabaseModifiedError.class DatabaseOpeningError.class DocNotFoundError.class\
 FeatureUnavailableError.class InternalError.class InvalidArgumentError.class\
 InvalidDataError.class InvalidOperationError.class InvalidResultError.class\
 LogicError.class NetworkError.class NetworkTimeoutError.class RangeError.class\
 RuntimeError.class TypeError.class UnimplementedError.class XapianError.class

all: $(XAPIAN_CLASS_FILES)

.java.class:
	$(JAVAC) -d . $<

CLEANFILES = $(XAPIAN_CLASS_FILES)
