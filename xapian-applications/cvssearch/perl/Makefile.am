bin_SCRIPTS = cvsbuild cvsupdatedb cvsquery-script cvssearch.pm cvscompare.cgi
EXTRA_DIST = cvsbuild.pl cvsupdatedb.pl cvsquery-script.pl cvscompare-script.pl cvssearch.pm cvssearch.conf

define build-perl-script
$(PERL) -c $(firstword $^)
rm -rf $@
echo "#! " $(PERL) > $@
cat $(firstword $^) >> $@
chmod ugo+rx $@
endef

cvsbuild: $(srcdir)/cvsbuild.pl
	$(build-perl-script)

cvsupdatedb: $(srcdir)/cvsupdatedb.pl
	$(build-perl-script)

cvsquery-script: $(srcdir)/cvsquery-script.pl
	$(build-perl-script)

cvscompare.cgi: $(srcdir)/cvscompare-script.pl
	$(build-perl-script)