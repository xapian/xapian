lib_LIBRARIES = libcvs_db.a
libcvs_db_a_SOURCES = cvs_comment_db.C     cvs_db.C       cvs_file_revision_db.C  cvs_line_db.C \
 cvs_comment_db.h     cvs_db.h       cvs_file_revision_db.h  cvs_line_db.h \
 cvs_comment_id_db.C  cvs_db_file.C  cvs_filename_db.C       cvs_revision_db.C \
 cvs_comment_id_db.h  cvs_db_file.h  cvs_filename_db.h       cvs_revision_db.h \
 cvs_revision_line_db.C cvs_revision_line_db.h \
 cvs_revision_less.h

bin_PROGRAMS = cvsindex cvssearch cvscomments cvssymbolcount cvscountsymbols cvsmine cvsquery #cvseval
bin_SCRIPTS = cvsbuild cvsupdatedb
EXTRA_DIST = cvsbuild.pl cvsupdatedb.pl

define build-perl-script
$(PERL) -c $(firstword $^)
rm -rf $@
echo "#! " $(PERL) > $@
cat $(firstword $^) >> $@
chmod ugo+rx $@
endef

cvsbuild : $(srcdir)/cvsbuild.pl
	$(build-perl-script)

cvsupdatedb : $(srcdir)/cvsupdatedb.pl
	$(build-perl-script)

cvsquery_SOURCES = cvsquery.C
cvsquery_LDADD = libcvs_db.a -ldb_cxx libcvs_db.a

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

#cvseval_SOURCES = util.h util.C cvseval.C
#cvseval_LDADD = -lomus -ldb_cxx

CXXFLAGS = -O3 -DINLINE -Wall

