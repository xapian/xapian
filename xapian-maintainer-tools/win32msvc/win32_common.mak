# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libcommon.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

OBJS= \
	$(INTDIR)\utils.obj \
	$(INTDIR)\getopt.obj \
	$(INTDIR)\omdebug.obj \
	$(INTDIR)\omstringstream.obj \
	$(INTDIR)\serialise-double.obj \
	$(INTDIR)\msvc_posix_wrapper.obj \
	$(INTDIR)\safe.obj
	
SRCS= \
	$(INTDIR)\utils.cc \
	$(INTDIR)\getopt.cc \
	$(INTDIR)\omdebug.cc \
	$(INTDIR)\omstringstream.cc \
	$(INTDIR)\serialise-double.cc \
	$(INTDIR)\msvc_posix_wrapper.cc \
	$(INTDIR)\safe.cc

CPP_PROJ=$(CPPFLAGS_EXTRA) -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL : "$(OUTDIR)\libcommon.lib" 
	
CLEAN :
	-@erase "$(OUTDIR)\libcommon.lib"
	-@erase "*.pch"
	-@erase "$(INTDIR)\*.pdb"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase $(OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(OUTDIR)\LIBCOMMON.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS) 
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libcommon.lib" $(DEF_FLAGS) $(OBJS)
<<

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

# Calculate any header dependencies and automatically insert them into this file
HEADERS :
            ..\win32\$(DEPEND) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)"
# DO NOT DELETE THIS LINE -- make depend depends on it.