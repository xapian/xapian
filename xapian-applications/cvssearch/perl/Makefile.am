bin_SCRIPTS = cvsbuilddb cvsupdatedb cvsquerydb	\
    Compare.cgi MatchFile.cgi Query.cgi QueryFile.cgi Source.cgi Top.cgi \
    Logo.cgi fishlogo.gif \
    PatternBrowser.cgi QueryCommit.cgi \
    MatchComment.cgi QueryComment.cgi TopComment.cgi SourceComment.cgi \
    cvs2cl \
    popup.js \
    Cvssearch.pm Entities.pm CommitSearch.cgi

CLEANFILES = cvsbuilddb cvsupdatedb cvsquerydb Compare.cgi MatchFile.cgi \
 Query.cgi QueryFile.cgi Source.cgi Top.cgi Logo.cgi PatternBrowser.cgi \
 QueryCommit.cgi MatchComment.cgi QueryComment.cgi TopComment.cgi \
 SourceComment.cgi cvs2cl CommitSearch.cgi

EXTRA_DIST = cvsbuild-script.pl cvsupdate-script.pl cvsquery-script.pl \
    Compare-script.pl \
    MatchFile-script.pl Query-script.pl QueryFile-script.pl Source-script.pl Top-script.pl Logo-script.pl \
    PatternBrowser-script.pl QueryCommit-script.pl \
    MatchComment-script.pl QueryComment-script.pl TopComment-script.pl SourceComment-script.pl \
    fishlogo.gif\
    cvs2cl.pl \
    popup.js \
    Cvssearch.pm Entities.pm CommitSearch-script.pl

cvs2cl: $(srcdir)/cvs2cl.pl
	rm -rf $@
	cp $(srcdir)/cvs2cl.pl $@
	chmod ugo+rx $@

# ----------------------------------------
# building databases
# ----------------------------------------
cvsbuilddb: $(srcdir)/cvsbuild-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/cvsbuild-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/cvsbuild-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

cvsupdatedb: $(srcdir)/cvsupdate-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/cvsupdate-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/cvsupdate-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

cvsquerydb: $(srcdir)/cvsquery-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/cvsquery-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/cvsquery-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

# ----------------------------------------
# cgi-scripts
# ----------------------------------------
Compare.cgi: $(srcdir)/Compare-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/Compare-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/Compare-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

MatchComment.cgi:$(srcdir)/MatchComment-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/MatchComment-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/MatchComment-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

QueryComment.cgi:$(srcdir)/QueryComment-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/QueryComment-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/QueryComment-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

TopComment.cgi:$(srcdir)/TopComment-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/TopComment-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/TopComment-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

MatchFile.cgi:$(srcdir)/MatchFile-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/MatchFile-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/MatchFile-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

Query.cgi:$(srcdir)/Query-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/Query-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/Query-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

QueryFile.cgi:$(srcdir)/QueryFile-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/QueryFile-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/QueryFile-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

SourceComment.cgi:$(srcdir)/SourceComment-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/SourceComment-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/SourceComment-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

Source.cgi:$(srcdir)/Source-script.pl 
	$(PERL) -I $(srcdir) -c $(srcdir)/Source-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/Source-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

Top.cgi:$(srcdir)/Top-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/Top-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/Top-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

Logo.cgi:$(srcdir)/Logo-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/Logo-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/Logo-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

PatternBrowser.cgi:$(srcdir)/PatternBrowser-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/PatternBrowser-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/PatternBrowser-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

QueryCommit.cgi:$(srcdir)/QueryCommit-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/QueryCommit-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/QueryCommit-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@

CommitSearch.cgi:$(srcdir)/CommitSearch-script.pl
	$(PERL) -I $(srcdir) -c $(srcdir)/CommitSearch-script.pl
	echo "#!$(PERL) -w" | cat $(srcdir)/CommitSearch-script.pl > tmp-$@
	chmod ugo+rx tmp-$@
	mv -f tmp-$@ $@
