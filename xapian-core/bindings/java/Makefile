# a list of source files

SRC   = com/muscat/om/OmObject.java			\
	com/muscat/om/OmReturnObject.java		\
	com/muscat/om/OmDatabaseGroup.java		\
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
	Test.java					

# classpath needed for compilation

CP    =  .:./classes:/usr/lib/jdk1.1/lib/classes.zip

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

