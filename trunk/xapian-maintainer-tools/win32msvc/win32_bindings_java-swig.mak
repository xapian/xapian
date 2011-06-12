# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd. www.lemurconsulting.com
# 7th April 2008

# Will build the Java Swig bindings 

# Where the core is, relative to the Java Swig bindings
# Change this to match your environment

XAPIAN_CORE_REL_JAVA=..\..\xapian-core
OUTLIBDIR=$(XAPIAN_CORE_REL_JAVA)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs

!INCLUDE $(XAPIAN_CORE_REL_JAVA)\win32\config.mak

OUTDIR=$(XAPIAN_CORE_REL_JAVA)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Java-Swig
INTDIR=.

XAPIAN_SWIG_JAVA_SRCS=\
	Auto.java\
	BM25Weight.java\
	BoolWeight.java\
    Brass.java\
    Chert.java\
	Database.java\
	DateValueRangeProcessor.java\
	Document.java\
	Enquire.java\
	ESet.java\
	ESetIterator.java\
	ExpandDecider.java\
	Flint.java\
	InMemory.java\
	MatchDecider.java\
	MSet.java\
	MSetIterator.java\
	MultiValueSorter.java\
	NumberValueRangeProcessor.java\
	PositionIterator.java\
	PostingIterator.java\
    PostingSource.java\
	Query.java\
	QueryParser.java\
	Remote.java\
	RSet.java\
	SimpleStopper.java\
	Sorter.java\
	Stem.java\
	Stopper.java\
	StringValueRangeProcessor.java\
	SWIGTYPE_p_std__string.java\
	TermGenerator.java\
	TermIterator.java\
	TradWeight.java\
	ValueIterator.java\
	ValueRangeProcessor.java\
	Version.java\
	Weight.java\
	WritableDatabase.java\
	Xapian.java\
	XapianConstants.java\
	XapianJNI.java

XAPIAN_SWIG_JAVA_CLASS = $(XAPIAN_SWIG_JAVA_SRCS:.java=.class)

# Java generates nested classes with filenames containing a $ (smart move) so
# we pick them up with a wildcard and omit them from dependencies to avoid
# escaping hell.  The lack of a dependency shouldn't really be an issue since
# these classes are always generated along with the containing class which
# is listed in the dependencies.
XAPIAN_SWIG_JAVA_EXTRA_CLASSES=\
	Enquire*docid_order.class\
	Query*op.class\
	QueryParser*feature_flag.class\
	QueryParser*stem_strategy.class\
	TermGenerator*flags.class
    
ALL : "$(OUTDIR)/xapian_jni.jar" \
      SmokeTest.class \
      "$(OUTDIR)/xapian_jni.dll" \
      WriteJavaVersion.class
# REMOVE THIS NEXT LINE if using Visual C++ .net 2003 - you won't need to worry about manifests. For later compilers this prevents error R6034
    $(MANIFEST) "$(OUTDIR)\xapian_jni.dll.manifest" -outputresource:"$(OUTDIR)\xapian_jni.dll;2"
    copy "$(ZLIB_LIB_DIR)\zdll.lib" 
    copy "$(ZLIB_BIN_DIR)\zlib1.dll" $(OUTDIR)

CLEAN :
    -@erase SmokeTest.class
    -@erase "$(OUTDIR)\*.class"
    -@erase "$(OUTDIR)\xapian_jni.jar"
    -@erase xapian_wrap.obj
    -@erase "$(OUTDIR)\xapian_jni.*"
    -@erase $(XAPIAN_SWIG_JAVA_CLASS)
    -@erase $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)
    -@erase Query*1.class
    -@erase *.pdb
    -@erase MyMatchDecider.class
    -@erase MyExpandDecider.class
    -@erase WriteJavaVersion.class
    -@erase version.res
    -@erase javaversion.h
    
CLEANSWIG: CLEAN
    -@erase xapian_wrap.cc
    -@erase xapian_wrap.h
    -@erase $(XAPIAN_SWIG_JAVA_SRCS)

DOTEST: 
    copy SmokeTest.class "$(OUTDIR)\SmokeTest.class"
    copy MyMatchDecider.class "$(OUTDIR)\MyMatchDecider.class"
    copy MyExpandDecider.class "$(OUTDIR)\MyExpandDecider.class"
    cd $(OUTDIR)
    $(JAVA) -classpath xapian_jni.jar; SmokeTest
    
CHECK: ALL DOTEST

DIST: CHECK 
    cd $(MAKEDIR)
    if not exist "$(OUTDIR)\dist\$(NULL)" mkdir "$(OUTDIR)\dist"
    if not exist "$(OUTDIR)\dist\docs/$(NULL)" mkdir "$(OUTDIR)\dist\docs"
    if not exist "$(OUTDIR)\dist\docs\examples/$(NULL)" mkdir "$(OUTDIR)\dist\docs\examples"        
    copy "$(OUTDIR)\xapian_jni.dll" "$(OUTDIR)\dist"
    copy "$(OUTDIR)\xapian_jni.jar" "$(OUTDIR)\dist"
    if exist docs copy docs\*.htm* "$(OUTDIR)\dist\docs"
    if exist docs\examples copy docs\examples\*.* "$(OUTDIR)\dist\docs\examples"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

   
$(OUTDIR)/xapian_jni.jar: xapian_wrap.obj "$(OUTDIR)" $(XAPIAN_SWIG_JAVA_CLASS)
	$(JAR) -cf $(OUTDIR)/xapian_jni.jar $(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES) 
    
CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_JAVA)" /I "$(XAPIAN_CORE_REL_JAVA)\include" /I "$(JAVA_INCLUDE_DIR)" /I "$(JAVA_INCLUDE_DIR)/win32"\
 /I"." /Fo"$(INTDIR)\\" /Tp$(INPUTNAME) 

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) 

"$(OUTDIR)\xapian_jni.dll" : "$(OUTDIR)" xapian_wrap.obj ".\version.res" 
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\xapian_jni.dll" xapian_wrap.obj ".\version.res" 

<<

!IF "$(SWIGBUILD)" == "1"
# FIXME: make this work properly with SWIG 
xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS): 
# Make sure that we don't package stale generated sources in the
# case where SWIG changes its mind as to which files it generates.
    -@erase $(XAPIAN_SWIG_JAVA_SRCS)
	$(SWIG) $(SWIG_FLAGS) -I$(XAPIAN_CORE_REL_JAVA)\include -I..\generic \
	    -c++ -java -module Xapian \
	    -o xapian_wrap.cc ../xapian.i   
# Insert code to automatically load the JNI library.
	$(PERL_EXE) -pe "print \"    System.loadLibrary('xapian_jni'); \n\" if /^\s*swig_module_init/" XapianJNI.java >XapianJNI.java.tmp
	-erase XapianJNI.java
	-rename XapianJNI.java.tmp XapianJNI.java
!ENDIF

JAVAOPTS=-classpath $(INTDIR) -d $(INTDIR) 

#
# Rules
#

".\version.res": version.rc WriteJavaVersion.class
    "$(JAVA)" WriteJavaVersion
    $(RSC) /v \
      /fo version.res \
      /I "$(XAPIAN_CORE_REL_JAVA)\include" \
      version.rc 

.java{$(XAPIAN_SWIG_JAVA_CLASS)}.class:
    $(JAVAC) $(JAVAOPTS) $*.java

xapian_wrap.obj : xapian_wrap.cc 
     $(CPP) @<<
  $(CPP_PROJ) $**
<<

SmokeTest.class: SmokeTest.java
    $(JAVAC) $(JAVAOPTS) SmokeTest.java
    
WriteJavaVersion.class: WriteJavaVersion.java
    $(JAVAC) $(JAVAOPTS) WriteJavaVersion.java