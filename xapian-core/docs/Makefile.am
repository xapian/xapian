### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

if HAVE_PERL

CREATED_DOCUMENTS1=code_structure.html

code_structure.html: mkdoc.pl
	@echo "Generating documentation"
	@chmod +x mkdoc.pl
	@./mkdoc.pl ${top_srcdir} $@

endif

if HAVE_DOCPP

CREATED_DOCUMENTS2=html/index.html

html/index.html:
	doc++ -j -A -c --dir html `find .. -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

endif

docdatadir   = ${datadir}/omuscat
docdata_DATA = ${CREATED_DOCUMENTS1} ${CREATED_DOCUMENTS2}

EXTRA_DIST = MatcherDesign Notes mkdoc.pl.in stemming.txt \
hacking.html index.html intro.html intro_ir.html
