### Process this file with automake to produce Makefile.in

DOCPP = @DOCPP_PATH@
DVIPS = @DVIPS_PATH@
LATEX = @LATEX_PATH@

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

HTMLFILES = index.html intro.html intro_ir.html userman.html \
        stemming.html porterstem.html using_stemmers.html

# Files which should be put in the distribution by automake
EXTRA_DIST = mkdoc.pl.in $(HTMLFILES) \
        MatcherDesign Notes stemming.txt \
	docxx.sty

# Things to do if have perl installed
if HAVE_PERL
CREATED_DOCUMENTS1=code_structure.html
endif

# Things to do if have latex installed
if HAVE_LATEX
all-dvi: apidoc.dvi sourcedoc.dvi
# Things to do if also have dvips installed
if HAVE_DVIPS
all-ps: apidoc.ps sourcedoc.ps
else
all-ps:
endif
else
all-dvi:
all-ps:
endif

# Things to do if have doc++ installed
if HAVE_DOCPP
all-local: apidoc/index.html sourcedoc/index.html all-dvi all-ps
dist-hook: all-local apidoc-dist
install-data-local: all-local apidoc-install
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
apidoc-dist: apidoc-html-dist apidoc-dvi-dist

apidoc-dvi-dist:
	@if [ -r apidoc.dvi ] ; then \
	  echo "Adding API DVI documentation to distribution" ; \
	  cp apidoc.dvi $(distdir)/ ; \
	else \
	  if [ -r $(srcdir)/apidoc.dvi ] ; then \
	    echo "Adding API DVI documentation to distribution" ; \
	    cp $(srcdir)/apidoc.dvi $(distdir)/ ; \
	  else \
	    echo "Skipping API DVI documentation from distribution: can't build" ; \
	  fi \
	fi

apidoc-html-dist:
	@if [ -r apidoc/index.html ] ; then \
	  echo "Adding API HTML documentation to distribution" ; \
	  $(mkinstalldirs) $(distdir)/apidoc ; \
	  cp apidoc/* $(distdir)/apidoc/ ; \
	else \
	  if [ -r $(srcdir)/apidoc/index.html ] ; then \
	    echo "Adding API HTML documentation to distribution" ; \
	    $(mkinstalldirs) $(distdir)/apidoc ; \
	    cp $(srcdir)/apidoc/* $(distdir)/apidoc/ ; \
	  else \
	    echo "Skipping API HTML documentation from distribution: can't build" ; \
	  fi \
	fi

#########################
# How to install apidoc #
#########################
apidoc-install: apidoc-html-install apidoc-dvi-install

apidoc-html-install:
	@if [ -r apidoc/index.html ] ; then \
	  echo "Installing API HTML documentation" ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	  $(INSTALL_DATA) apidoc/* $(DESTDIR)$(docdatadir)/apidoc/ ; \
	else \
	  if [ -r $(srcdir)/apidoc/index.html ] ; then \
	    echo "Installing API HTML documentation" ; \
	    $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	    $(INSTALL_DATA) $(srcdir)/apidoc/* $(DESTDIR)$(docdatadir)/apidoc/ ; \
	  else \
	    echo "NOT installing API HTML documentation: not present, and can't generate" ; \
	  fi \
	fi

apidoc-dvi-install:
	@if [ -r apidoc.dvi ] ; then \
	  echo "Installing API DVI documentation" ; \
	  $(INSTALL_DATA) apidoc.dvi $(DESTDIR)$(docdatadir)/ ; \
	else \
	  if [ -r $(srcdir)/apidoc.dvi ] ; then \
	    echo "Installing API DVI documentation" ; \
	    $(INSTALL_DATA) $(srcdir)/apidoc.dvi $(DESTDIR)$(docdatadir)/ ; \
	  else \
	    echo "NOT installing API DVI documentation: not present, and can't generate" ; \
	  fi \
	fi

###############################
# How to clean up local files #
###############################
clean-local:
	@if [ -d apidoc ]; then \
	  echo "removing apidoc/"; $(RM) apidoc/* ; rmdir apidoc ; \
	fi
	@if [ -r apidoc.tex ]; then \
	  echo "removing apidoc.*"; $(RM) apidoc.* ; \
	fi
	@if [ -d sourcedoc ]; then \
	  echo "removing sourcedoc/"; $(RM) sourcedoc/* ; rmdir sourcedoc ; \
	fi
	@if [ -r sourcedoc.tex ]; then \
	  echo "removing sourcedoc.*"; $(RM) sourcedoc.* ; \
	fi

################################
# How to uninstall local files #
################################
uninstall-local: 
	@if [ -d $(DESTDIR)$(docdatadir)/apidoc ]; then \
	  echo "Uninstalling $(DESTDIR)$(docdatadir)/apidoc" ; \
	  rm -rf $(DESTDIR)$(docdatadir)/apidoc ; \
	fi
	@if [ -e $(DESTDIR)$(docdatadir)/apidoc.dvi ]; then \
	  echo "Uninstalling $(DESTDIR)$(docdatadir)/apidoc.dvi" ; \
	  rm -rf $(DESTDIR)$(docdatadir)/apidoc.dvi ; \
	fi

###############
# Extra rules #
###############

code_structure.html: mkdoc.pl
	@echo "Generating documentation"
	@./mkdoc.pl ${top_srcdir} $@

DOCPPFLAGS = --sort --no-java-graphs --c-comments --quantel \
             --upwards-arrows --no-define

%.dvi: %.tex docxx.sty
	if [ ! -r docxx.sty ] ; then ln -s ${srcdir}/docxx.sty . ; fi
	$(LATEX) "$(basename $@)" || :
	$(LATEX) "$(basename $@)" || ( $(RM) $@ ; false )

%.ps: %.dvi
	$(DVIPS) -o $@ $<

apidoc/index.html:
	doc++ ${DOCPPFLAGS} --dir apidoc `find ${top_srcdir}/include -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

apidoc.tex:
	doc++ -t ${DOCPPFLAGS} --output $@ `find ${top_srcdir}/include -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

sourcedoc/index.html:
	doc++ ${DOCPPFLAGS} --private --dir sourcedoc `find ${top_srcdir} -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`

sourcedoc.tex:
	doc++ -t ${DOCPPFLAGS} --private --output $@ `find ${top_srcdir} -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c`
