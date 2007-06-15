# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libcommon.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

LIBCOMMON_OBJS= \
	$(INTDIR)\utils.obj \
	$(INTDIR)\getopt.obj \
	$(INTDIR)\omdebug.obj \
	$(INTDIR)\omstringstream.obj \
	$(INTDIR)\serialise-double.obj \
	$(INTDIR)\msvc_posix_wrapper.obj \
	$(INTDIR)\safe.obj
	
LOCAL_HEADERS =\
	$(INTDIR)\alltermslist.h\
	$(INTDIR)\autoptr.h\
	$(INTDIR)\database.h\
	$(INTDIR)\document.h\
	$(INTDIR)\documentterm.h\
	$(INTDIR)\emptypostlist.h\
	$(INTDIR)\expand.h\
	$(INTDIR)\expandweight.h\
	$(INTDIR)\gnu_getopt.h\
	$(INTDIR)\inmemory_positionlist.h\
	$(INTDIR)\leafpostlist.h\
	$(INTDIR)\msvc_posix_wrapper.h\
	$(INTDIR)\multialltermslist.h\
	$(INTDIR)\multimatch.h\
	$(INTDIR)\networkstats.h\
	$(INTDIR)\noreturn.h\
	$(INTDIR)\omassert.h\
	$(INTDIR)\omdebug.h\
	$(INTDIR)\omenquireinternal.h\
	$(INTDIR)\omqueryinternal.h\
	$(INTDIR)\omstringstream.h\
	$(INTDIR)\omtime.h\
	$(INTDIR)\output.h\
	$(INTDIR)\positionlist.h\
	$(INTDIR)\postlist.h\
	$(INTDIR)\progclient.h\
	$(INTDIR)\remoteconnection.h\
	$(INTDIR)\remote-database.h\
	$(INTDIR)\remoteprotocol.h\
	$(INTDIR)\remoteserver.h\
	$(INTDIR)\rset.h\
	$(INTDIR)\safeerrno.h\
	$(INTDIR)\safefcntl.h\
	$(INTDIR)\safesysselect.h\
	$(INTDIR)\safesysstat.h\
	$(INTDIR)\safeunistd.h\
	$(INTDIR)\safewindows.h\
	$(INTDIR)\safewinsock2.h\
	$(INTDIR)\serialise-double.h\
	$(INTDIR)\serialise.h\
	$(INTDIR)\stats.h\
	$(INTDIR)\submatch.h\
	$(INTDIR)\tcpclient.h\
	$(INTDIR)\tcpserver.h\
	$(INTDIR)\termlist.h\
	$(INTDIR)\utils.h\
	$(INTDIR)\vectortermlist.h

CPP_PROJ=$(CPPFLAGS_EXTRA) /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL : "$(OUTDIR)\libcommon.lib" 
	
CLEAN :
	-@erase "$(OUTDIR)\libcommon.lib"
	-@erase "*.pch"
	-@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBCOMMON_OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(OUTDIR)\LIBCOMMON.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBCOMMON_OBJS) 
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libcommon.lib" $(DEF_FLAGS) $(LIBCOMMON_OBJS)
<<

# if any headers change, rebuild all .objs
$(LIBCOMMON_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj: 
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
