bin_PROGRAMS = cvsindex cvssearch cvscomments

cvsindex_SOURCES = util.h util.C cvsindex.C
cvsindex_LDADD = -lomus -ldb_cxx

cvssearch_SOURCES = util.h util.C cvssearch.C
cvssearch_LDADD = -lomus

cvscomments_SOURCES = cvscomments.C
cvscomments_LDADD = -ldb_cxx

CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall

