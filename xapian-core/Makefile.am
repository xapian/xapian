## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I/usr/local/BerkeleyDB/include

noinst_HEADERS = da_database.h da_record.h daread.h database.h match.h \
 orpostlist.h andpostlist.h andnotpostlist.h andmaybepostlist.h \
 xorpostlist.h branchpostlist.h filterpostlist.h \
 damuscat.h omtypes.h proto_database.h proto_db.h sleepy_database.h \
 multi_database.h error.h stem.h irweight.h omassert.h

bin_PROGRAMS = dbtest matchtest idxtest stemtest

ACCESS_MULTI = multi_database.cc
ACCESS_SLEEPY = sleepy_database.cc
ACCESS_PROTO = proto_database.cc proto_db.c
ACCESS_DA = da_database.cc daread.c da_record.cc

STEMMER_SRC = stem.c stem_en.cc

dbtest_SOURCES = irweight.cc dbtest.cc $(ACCESS_DA) $(ACCESS_SLEEPY) \
                 $(ACCESS_MULTI)
dbtest_LDADD = -L/usr/local/BerkeleyDB/lib -ldb

matchtest_SOURCES = match.cc orpostlist.cc andpostlist.cc andnotpostlist.cc \
 andmaybepostlist.cc xorpostlist.cc matchtest.cc irweight.cc \
 $(ACCESS_DA) $(STEMMER_SRC)
matchtest_LDADD = -L/usr/local/BerkeleyDB/lib -ldb

idxtest_SOURCES = irweight.cc idxtest.cc $(ACCESS_SLEEPY) $(ACCESS_MULTI)
idxtest_LDADD = -L/usr/local/BerkeleyDB/lib -ldb

stemtest_SOURCES = $(STEMMER_SRC) stemtest.cc
