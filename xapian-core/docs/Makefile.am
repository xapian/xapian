### Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

HTMLFILES = index.html intro.html intro_ir.html userman.html \
        stemming.html porterstem.html using_stemmers.html

# Files which should be put in the distribution by automake
EXTRA_DIST = mkdoc.pl.in $(HTMLFILES) \
        MatcherDesign Notes stemming.txt

# Things to do if have perl installed
if HAVE_PERL
CREATED_DOCUMENTS1=code_structure.html
endif

# Things to do if have doc++ installed
if HAVE_DOCPP
all-local: apidoc/index.html sourcedoc/index.html
dist-hook: apidoc/index.html apidoc-dist
install-data-local: apidoc/index.html apidoc-install
else
all-local:
dist-hook: apidoc-dist
install-data-local: apidoc-install
endif

# Data to install, in the usual automake way
docdatadir   = $(datadir)/omuscat
docdata_DATA = $(CREATED_DOCUMENTS1) $(HTMLFILES)

#########################################
# How to put apidoc in the distribution #
#########################################
apidoc-dist:
	@if [ -r apidoc/index.html ] ; then \
	  echo "Adding API documentation to distribution" ; \
	  $(mkinstalldirs) $(distdir)/apidoc ; \
	  cp apidoc/* $(distdir)/apidoc/ ; \
	else \
	  if [ -r $(srcdir)/apidoc/index.html ] ; then \
	    echo "Adding API documentation to distribution" ; \
	    $(mkinstalldirs) $(distdir)/apidoc ; \
	    cp $(srcdir)/apidoc/* $(distdir)/apidoc/ ; \
	  else \
	    echo "Skipping API documentation from distribution: can't build" ; \
	  fi \
	fi

#########################
# How to install apidoc #
#########################
apidoc-install:
	@if [ -r apidoc/index.html ] ; then \
	  echo "Installing API documentation" ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	  $(INSTALL_DATA) apidoc/* $(DESTDIR)$(docdatadir)/apidoc/ ; \
	else \
	  if [ -r $(srcdir)/apidoc/index.html ] ; then \
	    echo "Installing API documentation" ; \
	    $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	    $(INSTALL_DATA) $(srcdir)/apidoc/* $(DESTDIR)$(docdatadir)/apidoc/ ; \
	  else \
	    echo "NOT installing API documentation: not present, and can't generate" ; \
	  fi \
	fi

###############################
# How to clean up local files #
###############################
clean-local:
	@if [ -d apidoc ]; then \
	  echo "removing apidoc/"; $(RM) apidoc/* ; rmdir apidoc ; \
	fi
	@if [ -d sourcedoc ]; then \
	  echo "removing sourcedoc/"; $(RM) sourcedoc/* ; rmdir sourcedoc ; \
	fi

################################
# How to uninstall local files #
################################
uninstall-local: 
	@echo "Uninstalling API documentation ($(DESTDIR)$(docdatadir)/apidoc)"
	@if [ -d $(DESTDIR)$(docdatadir)/apidoc ]; then \
	  rm -rf $(DESTDIR)$(docdatadir)/apidoc ; \
	fi

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

apidoc.tex:
	doc++ -t ${DOCPPFLAGS} --output $@ `find ${top_srcdir}/include -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

%.dvi: %.tex docxx.sty
	if [ ! -r docxx.sty ] ; then ln -s ${srcdir}/docxx.sty . ; fi
	latex "\\scrollmode\\input $(basename $@)" || :
	latex "\\scrollmode\\input $(basename $@)" || ( $(RM) $@ ; false )

sourcedoc/index.html:
	doc++ ${DOCPPFLAGS} --all --private --dir sourcedoc `find ${top_srcdir} -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

