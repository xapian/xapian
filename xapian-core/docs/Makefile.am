### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

if HAVE_PERL

CREATED_DOCUMENTS=code_structure.html

code_structure.html: mkdoc.pl
	@echo "Generating documentation"
	@chmod +x mkdoc.pl
	@./mkdoc.pl $(top_srcdir) $@

endif

pkgdata_DATA = $(CREATED_DOCUMENTS)
EXTRA_DIST = MatcherDesign Notes mkdoc.pl.in
