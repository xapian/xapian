jardir = $(top_srcdir)/java/built # directory to install xapian_jni.jar in

SUBDIRS = errors examples

SUFFIXES = .java .class

EXTRA_DIST = Database.java Document.java ESet.java ESetIterator.java\
 Enquire.java ExpandDecider.java MSet.java MSetIterator.java MatchDecider.java\
 PositionIterator.java Query.java RSet.java Stem.java TermIterator.java\
 WritableDatabase.java XapianJNI.java

XAPIAN_CLASS_FILES = Database.class Document.class ESet.class\
 ESetIterator.class Enquire.class ExpandDecider.class MSet.class\
 MSetIterator.class MatchDecider.class PositionIterator.class Query.class\
 RSet.class Stem.class TermIterator.class WritableDatabase.class\
 XapianJNI.class

jar_DATA = xapian_jni.jar

xapian.jar: $(XAPIAN_CLASS_FILES)
	$(JAR) -cf xapian_jni.jar $(XAPIAN_CLASS_FILES) errors/*.class

.java.class:
	$(JAVAC) -d . $<

CLEANFILES = $(XAPIAN_CLASS_FILES)

# something like this, but javadoc doesn't run on my system
#make-java-docs:
#	javadoc sourcepath=$(srcdir) destdir=docs packagenames='org.xapian.*'
