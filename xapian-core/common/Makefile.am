## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS = \
 omassert.h \
 database.h termlist.h postlist.h \
 leafpostlist.h \
 expandweight.h stats.h networkstats.h \
 match.h multimatch.h expand.h rset.h \
 utils.h omqueryinternal.h \
 document.h documentterm.h \
 progclient.h net_database.h \
 netutils.h omtime.h netserver.h progserver.h socketclient.h socketserver.h \
 socketcommon.h positionlist.h tcpclient.h \
 tcpserver.h omerr_string.h omdebug.h omlinebuf.h omstringstream.h \
 inmemory_positionlist.h omenquireinternal.h emptypostlist.h \
 autoptr.h \
 alltermslist.h multialltermslist.h \
 gnu_getopt.h safewindows.h safeerrno.h

noinst_LTLIBRARIES = libcommon.la

if BUILD_BACKEND_QUARTZ
remote_sources = netutils.cc
else
remote_sources =
endif

libcommon_la_SOURCES = utils.cc omdebug.cc omstringstream.cc $(remote_sources)
