bin_SCRIPTS = cvsbuilddb cvsupdatedb cvsquerydb	\
    Compare.cgi \
    MatchFile.cgi Query.cgi QueryFile.cgi Source.cgi Top.cgi Logo.cgi fishlogo.gif\
    MatchComment.cgi QueryComment.cgi TopComment.cgi \
    cvs2cl \
    popup.js \
    Cvssearch.pm Entities.pm 

EXTRA_DIST = cvsbuild-script.pl cvsupdate-script.pl cvsquery-script.pl \
    Compare-script.pl \
    MatchFile-script.pl Query-script.pl QueryFile-script.pl Source-script.pl Top-script.pl Logo-script.pl \
    MatchComment-script.pl QueryComment-script.pl TopComment-script.pl \
    fishlogo.gif\
    cvs2cl.pl \
    popup.js \
    Cvssearch.pm Entities.pm

define build-perl-script
$(PERL) -I $(srcdir) -c $(firstword $^)
rm -rf $@
echo "#!" $(PERL) " -w" > $@
cat $(firstword $^) >> $@
chmod ugo+rx $@
endef

cvs2cl: $(srcdir)/cvs2cl.pl
	rm -rf $@
	cat $(firstword $^) >> $@
	chmod ugo+rx $@

# ----------------------------------------
# building databases
# ----------------------------------------
cvsbuilddb: $(srcdir)/cvsbuild-script.pl 
	$(build-perl-script)

cvsupdatedb: $(srcdir)/cvsupdate-script.pl 
	$(build-perl-script)

cvsquerydb: $(srcdir)/cvsquery-script.pl 
	$(build-perl-script)

# ----------------------------------------
# cgi-scripts
# ----------------------------------------
Compare.cgi: $(srcdir)/Compare-script.pl 
	$(build-perl-script)

MatchComment.cgi:$(srcdir)/MatchComment-script.pl
	$(build-perl-script)

QueryComment.cgi:$(srcdir)/QueryComment-script.pl
	$(build-perl-script)

TopComment.cgi:$(srcdir)/TopComment-script.pl
	$(build-perl-script)

MatchFile.cgi:$(srcdir)/MatchFile-script.pl
	$(build-perl-script)

Query.cgi:$(srcdir)/Query-script.pl 
	$(build-perl-script)

QueryFile.cgi:$(srcdir)/QueryFile-script.pl 
	$(build-perl-script)

Source.cgi:$(srcdir)/Source-script.pl 
	$(build-perl-script)

Top.cgi:$(srcdir)/Top-script.pl
	$(build-perl-script)

Logo.cgi:$(srcdir)/Logo-script.pl
	$(build-perl-script)
