
SUBDIRS = . native

# classpath needed for compilation

CLASSPATH    =  $(top_builddir)/bindings/java:$(top_builddir)/bindings/java/classes:/usr/lib/jdk1.1/lib/classes.zip

JSRC  = com/muscat/om/OmObject.java			\
	com/muscat/om/OmReturnObject.java		\
	com/muscat/om/OmDatabaseGroup.java		\
	com/muscat/om/OmDatabase.java			\
	com/muscat/om/OmWritableDatabase.java		\
	com/muscat/om/OmDocumentContents.java		\
	com/muscat/om/OmEnquire.java			\
	com/muscat/om/OmDocument.java			\
	com/muscat/om/OmKey.java			\
	com/muscat/om/OmData.java			\
	com/muscat/om/OmMSet.java			\
	com/muscat/om/OmVector.java			\
	com/muscat/om/OmMSetItem.java			\
	com/muscat/om/OmQuery.java			\
	com/muscat/om/OmRSet.java			\
	com/muscat/om/OmMatchOptions.java		\
	com/muscat/om/OmMatchDecider.java		\
	com/muscat/om/OmVector.java			\
	com/muscat/om/OmStem.java			\
	com/muscat/om/OmMSetItemVector.java		\
	com/muscat/om/Mus36Field.java			\
	com/muscat/om/OmESet.java			\
	com/muscat/om/OmESetItem.java			\
	com/muscat/om/OmESetItemVector.java		\
	com/muscat/om/OmExpandOptions.java		\
	com/muscat/om/OmExpandDecider.java		\
	com/muscat/om/OmPrefixExpandDecider.java	\
	com/muscat/om/OmError.java			\
	com/muscat/om/OmRuntimeError.java		\
	com/muscat/om/OmOpeningError.java		\
	com/muscat/om/OmLogicError.java			\
	com/muscat/om/OmInvalidArgumentError.java	\
	com/muscat/om/OmDocNotFoundError.java		\
	com/muscat/om/OmNetworkError.java		\
	RunTest.java					\
	ApiTest.java					\
	TestFail.java					\
	ApiTestFuncs.java				\
	BackendManager.java

JCLASSES  = com/muscat/om/OmObject.class			\
	com/muscat/om/OmReturnObject.class		\
	com/muscat/om/OmDatabaseGroup.class		\
	com/muscat/om/OmDatabase.class 			\
	com/muscat/om/OmWritableDatabase.class 		\
	com/muscat/om/OmDocumentcontents.class 		\
	com/muscat/om/OmEnquire.class			\
	com/muscat/om/OmDocument.class			\
	com/muscat/om/OmKey.class			\
	com/muscat/om/OmData.class			\
	com/muscat/om/OmMSet.class			\
	com/muscat/om/OmVector.class			\
	com/muscat/om/OmMSetItem.class			\
	com/muscat/om/OmQuery.class			\
	com/muscat/om/OmRSet.class			\
	com/muscat/om/OmMatchOptions.class		\
	com/muscat/om/OmVector.class			\
	com/muscat/om/OmStem.class			\
	com/muscat/om/OmMSetItemVector.class		\
	com/muscat/om/Mus36Field.class			\
	com/muscat/om/OmESet.class			\
	com/muscat/om/OmESetItem.class			\
	com/muscat/om/OmESetItemVector.class		\
	com/muscat/om/OmExpandOptions.class		\
	com/muscat/om/OmExpandDecider.class		\
	com/muscat/om/OmPrefixExpandDecider.class	\
	com/muscat/om/OmError.class			\
	com/muscat/om/OmRuntimeError.class		\
	com/muscat/om/OmOpeningError.class		\
	com/muscat/om/OmLogicError.class			\
	com/muscat/om/OmInvalidArgumentError.class	\
	com/muscat/om/OmDocNotFoundError.class		\
	com/muscat/om/OmNetworkError.class		\
	RunTest.class                                   \
	ApiTest.class                                   \
	TestFail.class                                   \
	ApiTestFuncs.class				\
	BackendManager.class

# where to put the class files

JAVAROOT   =  $(top_builddir)/bindings/java/classes

# the java compiler

JAVAC = jikes
JAVACFLAGS = +E

# generate a list of class files

#TARGETS = $(foreach file,$(subst java,class,$(SRC)),$(OUT)/$(file))

# rule to make java -> class

ApiTestFuncs.java: $(top_srcdir)/tests/apitest.cc apitest_to_java.pl \
		   $(top_srcdir)/tests/apitest_parser.pm $(srcdir)/ToJava.pm
	@PERL_PATH@ -I$(srcdir)/ -I $(top_srcdir)/tests apitest_to_java.pl < $<

all-local: class.timestamp

class.timestamp: $(JSRC)
	CLASSPATH=$(CLASSPATH) $(JAVAC) -d $(JAVAROOT) $(JAVACFLAGS) $^
	echo "TIMESTAMP" > class.timestamp

#all:		$(TARGETS)

#clean:
#		rm -rf classes/*

#install:	all
#		cp -r classes/* /ferret/classes

