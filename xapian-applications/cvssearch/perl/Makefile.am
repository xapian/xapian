bin_SCRIPTS = cvsbuilddb cvsupdatedb cvsquerydb	\
    Compare.cgi \
    Match.cgi Query.cgi Queryfile.cgi Source.cgi Top.cgi\
    cvs2cl \
    popup.js utility.js \
    Cvssearch.pm Entities.pm logo.png

EXTRA_DIST = cvsbuild-script.pl cvsupdate-script.pl cvsquery-script.pl \
    Compare-script.pl \
    Match-script.pl Query-script.pl Queryfile-script.pl Source-script.pl Top-script.pl\
    cvs2cl.pl \
    popup.js utility.js \
    Cvssearch.pm Entities.pm logo.png

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

Match.cgi:$(srcdir)/Match-script.pl 
	$(build-perl-script)

Query.cgi:$(srcdir)/Query-script.pl 
	$(build-perl-script)

Queryfile.cgi:$(srcdir)/Queryfile-script.pl 
	$(build-perl-script)

Source.cgi:$(srcdir)/Source-script.pl 
	$(build-perl-script)

Top.cgi:$(srcdir)/Top-script.pl
	$(build-perl-script)
