bin_SCRIPTS = cvsbuilddb cvsupdatedb cvsquerydb \
	cvscompare.cgi \
	Match.cgi Query.cgi Queryfile.cgi Source.cgi \
        cvs_comment_extractor code_comment_extractor \
	popup.js utility.js \
	cvssearch.pm Entities.pm

EXTRA_DIST = cvsbuild-script.pl cvsupdate-script.pl cvsquery-script.pl \
	cvscompare-script.pl \
	Match-script.pl Query-script.pl Queryfile-script.pl Source-script.pl \
        comment_extractor_c.pl comment_extractor_c++.pl \
	popup.js utility.js \
        cvssearch.pm Entities.pm


define build-perl-script
$(PERL) -I $(srcdir) -c $(firstword $^)
rm -rf $@
echo "#!" $(PERL) > $@
cat $(firstword $^) >> $@
chmod ugo+rx $@
endef

cvsbuilddb: $(srcdir)/cvsbuild-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

cvsupdatedb: $(srcdir)/cvsupdate-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

cvsquerydb: $(srcdir)/cvsquery-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

cvscompare.cgi: $(srcdir)/cvscompare-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

Match.cgi:$(srcdir)/Match-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

Query.cgi:$(srcdir)/Query-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

Queryfile.cgi:$(srcdir)/Queryfile-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

Source.cgi:$(srcdir)/Source-script.pl $(srcdir)/Entities.pm $(srcdir)/cvssearch.pm
	$(build-perl-script)

cvs_comment_extractor: $(srcdir)/cvs_comment_extractor.pl
	$(build-perl-script)

code_comment_extractor: $(srcdir)/code_comment_extractor.pl
	$(build-perl-script)


