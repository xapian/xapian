### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

if HAVE_PERL

CREATED_DOCUMENTS1=code_structure.html

endif

if HAVE_DOCPP

CREATED_DOCUMENTS2=apidoc/index.html # sourcedoc/index.html

all-local: apidoc/index.html sourcedoc/index.html

dist-hook: apidoc/index.html
	mkdir $(distdir)/apidoc
	if [ -r apidoc/index.html ] ; then \
	  cp apidoc/* $(distdir)/apidoc/ ;\
	else \
	  cp $(srcdir)/apidoc/* $(distdir)/apidoc/ ;\
	fi

install-data-hook: apidoc/index.html
	mkdir $(DESTDIR)$(datadir)/omuscat/apidoc
	if [ -r apidoc/index.html ] ; then \
	  cp apidoc/* $(DESTDIR)$(datadir)/omuscat/apidoc/ ; \
	else \
	  cp $(srcdir)/apidoc/* $(DESTDIR)$(datadir)/omuscat/apidoc/ ; \
	fi

else
dist-hook:
install-data-hook:

endif

docdatadir   = $(datadir)/omuscat
docdata_DATA = $(CREATED_DOCUMENTS1)

EXTRA_DIST = MatcherDesign Notes mkdoc.pl.in stemming.txt \
hacking.html index.html intro.html intro_ir.html

###############
# Extra rules #
###############

code_structure.html: mkdoc.pl
	@echo "Generating documentation"
	@./mkdoc.pl ${top_srcdir} $@

DOCPPFLAGS = --sort --no-java-graphs --c-comments --quantel \
             --upwards-arrows --no-define

apidoc/index.html:
	doc++ ${DOCPPFLAGS} --dir apidoc `find ${top_srcdir}/include -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

sourcedoc/index.html:
	doc++ ${DOCPPFLAGS} --all --private --dir sourcedoc `find ${top_srcdir} -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

