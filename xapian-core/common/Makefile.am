## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = \
 omassert.h \
 database_builder.h database.h termlist.h postlist.h \
 leafpostlist.h \
 irweight.h expandweight.h stats.h networkstats.h \
 match.h localmatch.h networkmatch.h multimatch.h expand.h rset.h \
 stemmer.h \
 indexer.h textfile_indexer.h \
 utils.h omlocks.h omqueryinternal.h omrefcnt.h \
 omdocumentparams.h document.h networkmatch.h \
 netclient.h progclient.h multi_database.h net_database.h \
 omdatabaseinterface.h \
 netutils.h netserver.h progserver.h socketclient.h socketserver.h \
 socketcommon.h positionlist.h testsuite.h testutils.h tcpclient.h \
 tcpserver.h omerr_string.h omdebug.h

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = utils.cc omdebug.cc
