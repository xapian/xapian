bin_PROGRAMS = cvsindex cvssearch cvscomments cvssymbolcount cvscountsymbols cvsmine

cvsindex_SOURCES = util.h util.C cvsindex.C
cvsindex_LDADD = -lomus -ldb_cxx

cvssearch_SOURCES = util.h util.C cvssearch.C
cvssearch_LDADD = -lomus

cvscomments_SOURCES = cvscomments.C
cvscomments_LDADD = -ldb_cxx -lomus

cvssymbolcount_SOURCES = cvssymbolcount.C
cvssymbolcount_LDADD = -ldb_cxx -lomus

cvscountsymbols_SOURCES = util.h util.C cvscountsymbols.C
cvscountsymbols_LDADD = -ldb_cxx -lomus

cvsmine_SOURCES = util.h util.C cvsmine.C
cvsmine_LDADD = -lomus -ldb_cxx

CXXFLAGS = -O3 -DINLINE -Wall

