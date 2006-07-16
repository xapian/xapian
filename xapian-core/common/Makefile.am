## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS = serialise.h \
 omassert.h \
 database.h termlist.h postlist.h \
 leafpostlist.h \
 expandweight.h stats.h networkstats.h \
 multimatch.h submatch.h expand.h rset.h \
 utils.h omqueryinternal.h \
 document.h documentterm.h \
 progclient.h remote-database.h \
 omtime.h remoteserver.h \
 remoteconnection.h positionlist.h tcpclient.h \
 tcpserver.h omdebug.h omstringstream.h \
 inmemory_positionlist.h omenquireinternal.h emptypostlist.h \
 autoptr.h \
 alltermslist.h multialltermslist.h \
 gnu_getopt.h safewindows.h safeerrno.h

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = utils.cc omdebug.cc omstringstream.cc c_strtod.cc
