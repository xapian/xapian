# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 3rd Jan 2007


# Will build a Win32 static library (non-debug) libremote.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libremote.lib" 

OBJS= \
                 $(INTDIR)\remote-database.obj \
                 $(INTDIR)\remote-document.obj \
                 $(INTDIR)\net_termlist.obj \
                 $(INTDIR)\net_postlist.obj \
				 
SRCS= \
                 $(INTDIR)\remote-database.cc \
                 $(INTDIR)\remote-document.cc \
                 $(INTDIR)\net_termlist.cc \
                 $(INTDIR)\net_postlist.cc \

	
CLEAN :
	-@erase "$(OUTDIR)\libremote.lib"
	-@erase "*.pch"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\*.pdb"
	-@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 -I "..\.." -I "..\..\include" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME) 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBREMOTE.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libremote.lib" $(DEF_FLAGS) $(OBJS)
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
    if exist "..\..\win32\$(DEPEND)" ..\..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.