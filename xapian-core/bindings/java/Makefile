# a list of source files

SRC   = com/webtop/om/OmObject.java			\
	com/webtop/om/OmReturnObject.java		\
	com/webtop/om/OmDatabaseGroup.java		\
	com/webtop/om/OmEnquire.java			\
	com/webtop/om/OmDocument.java			\
	com/webtop/om/OmKey.java			\
	com/webtop/om/OmData.java			\
	com/webtop/om/OmMSet.java			\
	com/webtop/om/OmVector.java			\
	com/webtop/om/OmMSetItem.java			\
	com/webtop/om/OmQuery.java			\
	com/webtop/om/OmRSet.java			\
	com/webtop/om/OmMatchOptions.java		\
	com/webtop/om/OmVector.java			\
	com/webtop/om/OmStem.java			\
	com/webtop/om/OmMSetItemVector.java		\
	com/webtop/om/Mus36Field.java			\
	com/webtop/om/OmESet.java			\
	com/webtop/om/OmESetItem.java			\
	com/webtop/om/OmESetItemVector.java		\
	com/webtop/om/OmExpandOptions.java		\
	com/webtop/om/OmExpandDecider.java		\
	com/webtop/om/OmPrefixExpandDecider.java	\
	com/webtop/format/FerretUnpacker.java		\
	com/webtop/format/RecordFormatter.java		\
	com/webtop/format/FerretFormatter.java		\
	com/webtop/om/OmError.java			\
	com/webtop/om/OmRuntimeError.java		\
	com/webtop/om/OmOpeningError.java		\
	com/webtop/om/OmLogicError.java			\
	com/webtop/om/OmInvalidArgumentError.java	\
	com/webtop/om/OmDocNotFoundError.java		\
	com/webtop/om/OmNetworkError.java		\
	Test.java					

# classpath needed for compilation

CP    =  .:./classes:/usr/local/jdk/jre/lib/rt.jar

# where to put the class files

OUT   =  ./classes

# the java compiler

JAVAC = jikes +E

# generate a list of class files

TARGETS = $(foreach file,$(subst java,class,$(SRC)),$(OUT)/$(file))

# rule to make java -> class

$(OUT)/%.class:	%.java
		CLASSPATH=$(CP) $(JAVAC) -d $(OUT) $<

all:		$(TARGETS)

clean:
		rm -rf classes/*

install:	all
		cp -r classes/* /ferret/classes

