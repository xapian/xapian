# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Mark Hammond

# Will build a Win32 static library (non-debug) libunicode.lib



!INCLUDE ..\win32\config.mak


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libunicode.lib" 

LIBUNICODE_OBJS= \
            $(INTDIR)\utf8itor.obj \
            $(INTDIR)\tclUniData.obj
	    
LOCAL_HEADERS= 	    

CLEAN :
	-@erase "$(OUTDIR)\libunicode.lib"
	-@erase "*.pch"
        -@erase $(LIBUNICODE_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\api" /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\libunicode.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBUNICODE_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libunicode.lib" $(DEF_FLAGS) $(LIBUNICODE_OBJS)
<<


# if any headers change, rebuild all .objs
$(LIBUNICODE_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

