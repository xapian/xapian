# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libbtreecheck.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

BUILD_LIBRARIES = "$(OUTDIR)\libtest.lib"

ALL : $(BUILD_LIBRARIES) 

LIBTEST_OBJS= \
                $(INTDIR)\testsuite.obj \
                $(INTDIR)\testutils.obj \
                $(INTDIR)\backendmanager.obj \
                $(INTDIR)\index_utils.obj \
		$(INTDIR)\unixcmds.obj

LOCAL_HEADERS= \
                $(INTDIR)\testsuite.h \
                $(INTDIR)\testutils.h \
                $(INTDIR)\backendmanager.h \
                $(INTDIR)\index_utils.h \
		$(INTDIR)\unixcmds.h
CLEAN :
	-@erase $(BUILD_LIBRARIES)
	-@erase "*.pch"
        -@erase $(LIBTEST_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\.." /I"..\..\include" /I"..\..\api" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBTEST.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBTEST_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libtest.lib" $(DEF_FLAGS) $(LIBTEST_OBJS)
<<


# if any headers change, rebuild all .objs
$(LIBTEST_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

