SUBDIRS = errors . examples

SUFFIXES = .java .class

EXTRA_DIST = Database.java Document.java ESet.java ESetIterator.java\
 Enquire.java ExpandDecider.java MSet.java MSetIterator.java MatchDecider.java\
 PositionIterator.java Query.java RSet.java Stem.java TermIterator.java\
 WritableDatabase.java Xapian.java XapianJNI.java

XAPIAN_CLASS_FILES = Database.class Document.class ESet.class\
 ESetIterator.class Enquire.class ExpandDecider.class MSet.class\
 MSetIterator.class MatchDecider.class PositionIterator.class Query.class\
 RSet.class Stem.class TermIterator.class WritableDatabase.class\
 Xapian.class XapianJNI.class

# Specify these separately from XAPIAN_CLASS_FILES because they're not so easy
# to write a rule which generates them, they get generated as a side effect of
# generating the files in XAPIAN_CLASS_FILES, and the only thing we really want
# to do with them is clean them.
INNERCLASS_FILES = TermIterator\$$Info.class Xapian\$$Auto.class\
 Xapian\$$InMemory.class Xapian\$$Muscat36.class Xapian\$$Quartz.class\
 Xapian\$$Remote.class

noinst_DATA = $(XAPIAN_CLASS_FILES)

.java.class:
	-rm -f ../../built/xapian_jni.jar
	$(JAVAC) -classpath $(srcdir)/../..$(JAVA_PATHSEP)../.. -d ../.. $<

CLEANFILES = $(XAPIAN_CLASS_FILES) $(INNERCLASS_FILES)

# something like this, but javadoc doesn't run on my system
#make-java-docs:
#	javadoc sourcepath=$(srcdir) destdir=docs packagenames='org.xapian.*'
