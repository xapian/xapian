### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

if HAVE_PERL

CREATED_DOCUMENTS=code_structure.html

code_structure.html: mkdoc.pl
	@echo "Generating documentation"
	@./mkdoc.pl ${top_srcdir} $@

endif

docdatadir   = ${datadir}/omuscat
docdata_DATA = ${CREATED_DOCUMENTS}

EXTRA_DIST = MatcherDesign Notes mkdoc.pl.in stemming.txt \
hacking.html index.html intro.html intro_ir.html
