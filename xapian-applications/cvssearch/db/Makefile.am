bin_PROGRAMS = cvsindex cvssearch cvscomments cvssymbolcount cvscountsymbols

cvsindex_SOURCES = util.h util.C cvsindex.C
cvsindex_LDADD = -lomus -ldb_cxx

cvssearch_SOURCES = util.h util.C cvssearch.C
cvssearch_LDADD = -lomus

cvscomments_SOURCES = cvscomments.C
cvscomments_LDADD = -ldb_cxx

cvssymbolcount_SOURCES = cvssymbolcount.C
cvssymbolcount_LDADD = -ldb_cxx

cvscountsymbols_SOURCES = util.h util.C cvscountsymbols.C
cvscountsymbols_LDADD = -ldb_cxx

CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall

