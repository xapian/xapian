## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = da_database.h daread.h database.h match.h mergedpostlist.h \
 muscat.h omtypes.h proto_database.h proto_db.h

bin_PROGRAMS = dbtest matchtest

ACCESS_PROTO = proto_database.cc proto_db.c
ACCESS_DA = da_database.cc trial.cc

dbtest_SOURCES = dbtest.cc $(ACCESS_PROTO) $(ACCESS_DA)
dbtest_LDADD = -L/usr/local/BerkeleyDB/lib -ldb

matchtest_SOURCES = match.cc mergedpostlist.cc matchtest.cc $(ACCESS_PROTO) $(ACCESS_DA)
matchtest_LDADD = -L/usr/local/BerkeleyDB/lib -ldb
