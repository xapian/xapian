bin_SCRIPTS = cvsbuilddb cvsupdatedb cvsquerydb \
	cvscompare.cgi \
	Match.cgi Query.cgi Queryfile.cgi Source.cgi \
	comment_extractor_c comment_extractor_c++ \
	popup.js utility.js \
	cvssearch.pm Entities.pm

EXTRA_DIST = cvsbuild-script.pl cvsupdatedb-script.pl cvsquery-script.pl \
	cvscompare-script.pl cvssearch.pm \
	Match-script.pl Query-script.pl Queryfile-script.pl Source-script.pl comment_extractor_c.pl comment_extractor_c++.pl

define build-perl-script
$(PERL) -c $(firstword $^)
rm -rf $@
echo "#! " $(PERL) > $@
cat $(firstword $^) >> $@
chmod ugo+rx $@
endef

cvsbuilddb: $(srcdir)/cvsbuild-script.pl
	$(build-perl-script)

cvsupdatedb: $(srcdir)/cvsupdate-script.pl
	$(build-perl-script)

cvsquerydb: $(srcdir)/cvsquery-script.pl
	$(build-perl-script)

cvscompare.cgi: $(srcdir)/cvscompare-script.pl
	$(build-perl-script)

Match.cgi:$(srcdir)/Match-script.pl
	$(build-perl-script)

Query.cgi:$(srcdir)/Query-script.pl
	$(build-perl-script)

Queryfile.cgi:$(srcdir)/Queryfile-script.pl
	$(build-perl-script)

Source.cgi:$(srcdir)/Source-script.pl
	$(build-perl-script)

comment_extractor_c: $(srcdir)/comment_extractor_c.pl
	$(build-perl-script)

comment_extractor_c++:$(srcdir)/comment_extractor_c++.pl
	$(build-perl-script)
