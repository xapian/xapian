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
    $(INTDIR)\bitstream.obj\
    $(INTDIR)\const_database_wrapper.obj\
    $(INTDIR)\debuglog.obj\
    $(INTDIR)\fileutils.obj \
    $(INTDIR)\getopt.obj \
    $(INTDIR)\io_utils.obj \
    $(INTDIR)\msvc_dirent.obj \
    $(INTDIR)\msvc_posix_wrapper.obj \
    $(INTDIR)\omdebug.obj \
    $(INTDIR)\replicate_utils.obj \
    $(INTDIR)\safe.obj \
    $(INTDIR)\serialise-double.obj \
    $(INTDIR)\socket_utils.obj \
    $(INTDIR)\str.obj\
    $(INTDIR)\stringutils.obj \
    $(INTDIR)\utils.obj \
    $(INTDIR)\win32_uuid.obj 
  
SRCS= \
    $(INTDIR)\bitstream.cc\
    $(INTDIR)\const_database_wrapper.cc\
    $(INTDIR)\debuglog.cc\
    $(INTDIR)\fileutils.cc \
    $(INTDIR)\getopt.cc \
    $(INTDIR)\io_utils.cc \
    $(INTDIR)\msvc_dirent.cc \
    $(INTDIR)\msvc_posix_wrapper.cc \
    $(INTDIR)\omdebug.cc \
    $(INTDIR)\replicate_utils.cc \
    $(INTDIR)\safe.cc \
    $(INTDIR)\serialise-double.cc \
    $(INTDIR)\socket_utils.cc \
    $(INTDIR)\str.cc\
    $(INTDIR)\stringutils.cc \
    $(INTDIR)\utils.cc \
    $(INTDIR)\win32_uuid.cc 

   
CPP_PROJ=$(CPPFLAGS_EXTRA) -I..\win32\ -Fo"$(INTDIR)\\" -Tp$(INPUTNAME) 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL : "$(OUTDIR)\libcommon.lib" 

CLEAN :
    -@erase "$(OUTDIR)\libcommon.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase $(OBJS)
    -@erase deps.d

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
    -@erase deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(SRCS) >>deps.d
    if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.

