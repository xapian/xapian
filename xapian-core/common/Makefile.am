## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = \
 omassert.h \
 database_builder.h database.h termlist.h postlist.h \
 leafpostlist.h \
 irweight.h expandweight.h stats.h \
 match.h leafmatch.h networkmatch.h multimatch.h expand.h rset.h \
 stemmer.h \
 indexer.h textfile_indexer.h query_parser.h \
 utils.h omlocks.h omqueryinternal.h omrefcnt.h \
 omdocumentparams.h document.h networkmatch.h \
 netclient.h progclient.h multi_database.h net_database.h

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = utils.cc
