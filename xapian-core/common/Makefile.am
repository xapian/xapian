## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

noinst_HEADERS = \
 omassert.h \
 database_builder.h database.h termlist.h postlist.h \
 leafpostlist.h \
 irweight.h expandweight.h stats.h networkstats.h \
 match.h localmatch.h networkmatch.h multimatch.h expand.h rset.h \
 indexer.h textfile_indexer.h \
 utils.h omlocks.h omqueryinternal.h refcnt.h \
 omdocumentparams.h document.h networkmatch.h \
 netclient.h progclient.h net_database.h \
 netutils.h netserver.h progserver.h socketclient.h socketserver.h \
 socketcommon.h positionlist.h tcpclient.h \
 tcpserver.h omerr_string.h omdebug.h omlinebuf.h omstringstream.h \
 inmemory_positionlist.h omenquireinternal.h emptypostlist.h \
 omdatabaseinterface.h

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = utils.cc omdebug.cc omstringstream.cc
