# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull
# 12th December 2008

# Will build the Csharp  bindings 

# Where the core is, relative to the Csharp bindings
# Change this to match your environment

XAPIAN_CORE_REL_CSHARP=..\..\xapian-core
OUTLIBDIR=$(XAPIAN_CORE_REL_CSHARP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs

!INCLUDE $(XAPIAN_CORE_REL_CSHARP)\win32\config.mak

OUTDIR=$(XAPIAN_CORE_REL_CSHARP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\CSharp
INTDIR=.

# Note we have two DLLs: the Assembly is built by the Csharp compiler and references the Binding which is built by the C compiler
ASSEMBLY=XapianCSharp
BINDING=_XapianSharp

XAPIAN_SWIG_CSHARP_SRCS=\
    Auto.cs \
    BM25Weight.cs \
    BoolWeight.cs \
    Brass.cs \
    Compactor.cs \
    Chert.cs \
    Database.cs \
    DateValueRangeProcessor.cs \
    Document.cs \
    Enquire.cs \
    ESet.cs \
    ESetIterator.cs \
    ExpandDecider.cs \
    Flint.cs \
    InMemory.cs \
    KeyMaker.cs \
    MatchSpy.cs \
    MatchDecider.cs \
    MSet.cs \
    MSetIterator.cs \
    MultiValueSorter.cs \
    NumberValueRangeProcessor.cs \
    PositionIterator.cs \
    PostingIterator.cs \
    PostingSource.cs \
    Query.cs \
    QueryParser.cs \
    Registry.cs \
    Remote.cs \
    RSet.cs \
    SimpleStopper.cs \
    SmokeTest.cs \
    Sorter.cs \
    Stem.cs \
    StemImplementation.cs \
    Stopper.cs \
    StringValueRangeProcessor.cs \
    SWIGTYPE_p_std__string.cs \
    SWIGTYPE_p_std__vectorT_std__string_t.cs \
    SWIGTYPE_p_std__vectorT_Xapian__Query_t.cs \
    TermGenerator.cs \
    TermIterator.cs \
    TradWeight.cs \
    ValueIterator.cs \
    ValueRangeProcessor.cs \
    Version.cs \
    Weight.cs \
    WritableDatabase.cs \
    Xapian.cs \
    XapianPINVOKE.cs
    
ALL : "$(ASSEMBLY).dll" SmokeTest.exe "$(BINDING).dll"
# REMOVE THIS NEXT LINE if using Visual C++ .net 2003 - you won't need to worry about manifests. For later compilers this prevents error R6034
    $(MANIFEST) "$(BINDING).dll.manifest" -outputresource:"$(BINDING).dll;2"
    copy  "$(ASSEMBLY).dll" $(OUTDIR)
    copy  "$(BINDING).dll" $(OUTDIR)
    copy "$(ZLIB_LIB_DIR)\zdll.lib" 
    copy "$(ZLIB_BIN_DIR)\zlib1.dll" $(OUTDIR)

CLEAN :
    -@erase XapianSharp.snk 
    -@erase AssemblyInfo.cs
    -@erase "$(BINDING).dll"
    -@erase "$(OUTDIR)\$(BINDING).dll"
    -@erase "$(BINDING).dll.manifest"
    -@erase "$(ASSEMBLY).dll" 
    -@erase "$(OUTDIR)\$(ASSEMBLY).dll"
    -@erase "$(ASSEMBLY).dll.manifest" 
    -@erase "SmokeTest.exe"
    -@erase "$(OUTDIR)\SmokeTest.exe"
    -@erase xapian_wrap.obj
    -@erase *.pdb
    -@erase *.ilk
    -@erase *.lib
    -@erase *.exp
    -@erase version.res
    
CLEANSWIG: CLEAN
    -@erase xapian_wrap.cc
    -@erase xapian_wrap.h
    -@erase $(XAPIAN_SWIG_CSHARP_SRCS)

DOTEST: 
    copy SmokeTest.exe "$(OUTDIR)\SmokeTest.exe"
    cd $(OUTDIR)
    SmokeTest
    
CHECK: ALL DOTEST

DIST: CHECK 
    cd $(MAKEDIR)
    if not exist "$(OUTDIR)\dist\$(NULL)" mkdir "$(OUTDIR)\dist"
    if not exist "$(OUTDIR)\dist\docs/$(NULL)" mkdir "$(OUTDIR)\dist\docs"
    if not exist "$(OUTDIR)\dist\docs\examples/$(NULL)" mkdir "$(OUTDIR)\dist\docs\examples"           
    copy "$(OUTDIR)\_XapianSharp.dll" "$(OUTDIR)\dist"
    copy "$(OUTDIR)\XapianCSharp.dll" "$(OUTDIR)\dist"
    if exist docs copy docs\*.htm* "$(OUTDIR)\dist\docs"
    if exist docs\examples copy docs\examples\*.* "$(OUTDIR)\dist\docs\examples"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

XapianSharp.snk:
    $(SN) -k $@
    
"$(ASSEMBLY).dll": $(XAPIAN_SWIG_CSHARP_SRCS) AssemblyInfo.cs XapianSharp.snk "$(OUTDIR)" $(BINDING).dll 
    $(CSC) -unsafe -target:library -out:"$(ASSEMBLY).dll" \
        $(XAPIAN_SWIG_CSHARP_SRCS) AssemblyInfo.cs  

AssemblyInfo.cs: AssemblyInfo.cs.in "$(XAPIAN_CORE_REL_CSHARP)\configure.ac"
    $(PERL_EXE) "$(XAPIAN_CORE_REL_CSHARP)\win32\genversion.pl"  "$(XAPIAN_CORE_REL_CSHARP)\configure.ac" AssemblyInfo.cs.in AssemblyInfo.cs
        
CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_CSHARP)" /I "$(XAPIAN_CORE_REL_CSHARP)\include" \
 /I"." /Fo"$(INTDIR)\\" /Tp$(INPUTNAME) 
 
ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) 

"$(BINDING).dll" : "$(OUTDIR)" xapian_wrap.obj ".\version.res" 
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(BINDING).dll" xapian_wrap.obj ".\version.res" 

<<
  
!IF "$(SWIGBUILD)" == "1"
xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_CSHARP_SRCS): util.i ..\xapian.i
# Make sure that we don't package stale generated sources in the
# case where SWIG changes its mind as to which files it generates.
    -@erase $(XAPIAN_SWIG_CSHARP_SRCS)
    $(SWIG) $(SWIG_FLAGS) -I$(XAPIAN_CORE_REL_CSHARP)\include -I..\generic \
        -csharp -namespace Xapian -module Xapian -dllimport $(BINDING) \
        -c++ -o xapian_wrap.cc ..\xapian.i   
!ENDIF

#
# Rules
#

".\version.res": version.rc
    $(RSC) /v \
      /I "$(XAPIAN_CORE_REL_CSHARP)\include" \
      /fo version.res \
      version.rc 
      
xapian_wrap.obj : xapian_wrap.cc 
     $(CPP) @<<
  $(CPP_PROJ) $**
<<

SmokeTest.exe: SmokeTest.cs $(ASSEMBLY).dll $(BINDING).dll
	$(CSC) -unsafe -target:exe -out:SmokeTest.exe SmokeTest.cs -r:$(ASSEMBLY).dll
