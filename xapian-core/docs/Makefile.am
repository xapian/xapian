## Process this file with automake to produce Makefile.in

DOCPP = @DOCPP_PATH@
DVIPS = @DVIPS_PATH@
LATEX = @LATEX_PATH@

HTMLFILES = index.html intro.html intro_ir.html userman.html \
        stemming.html porterstem.html using_stemmers.html quickstart.html

# Files which should be put in the distribution by automake
EXTRA_DIST = mkdoc.pl.in todo2html.pl.in \
        doxygen_api_conf.in doxygen_full_conf.in \
        $(HTMLFILES) \
        MatcherDesign Notes stemming.txt \
	docxx.sty \
	doxygen_api_header.html_tmpl \
	doxygen_api_footer.html_tmpl \
	doxygen_full_header.html_tmpl \
	doxygen_full_footer.html_tmpl

# Things to do if have perl installed
if HAVE_PERL
CREATED_DOCUMENTS1=code_structure.html todo.html
endif

# Things to do if have LaTeX installed
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

# Things to do if have doxygen installed 
if HAVE_DOXYGEN
all-local: # doxygen_docs
dist-hook: doxygen_docs doxygen_dist
install-data-local: doxygen_docs doxygen_install
else
all-local:
dist-hook: doxygen_dist
install-data-local: doxygen_install
endif

# Data to install, in the usual automake way
docdatadir   = $(datadir)/omuscat
docdata_DATA = $(CREATED_DOCUMENTS1) $(HTMLFILES)

###############################################
# How to put doxygen docs in the distribution #
###############################################
doxygen_dist: doxygen_html_dist

doxygen_html_dist:
	@if [ -r apidoc/html/index.html ] ; then \
	  echo "Adding API documentation to distribution" ; \
	  $(mkinstalldirs) $(distdir)/apidoc ; \
	  cp -a apidoc/* $(distdir)/apidoc/ ; \
	else \
	  echo "Skipping API documentation from distribution: can't build" ; \
	fi

#########################
# How to install apidoc #
#########################
doxygen_install: doxygen_html_install

doxygen_html_install:
	@if [ -r apidoc/html/index.html ] ; then \
	  echo "Installing API HTML documentation" ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc/html ; \
	  $(INSTALL_DATA) apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	else \
	  if [ -r $(srcdir)/apidoc/html/index.html ] ; then \
	    echo "Installing API HTML documentation" ; \
	    $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	    $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc/html ; \
	    $(INSTALL_DATA) $(srcdir)/apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	  else \
	    echo "NOT installing API HTML documentation: not present, and can't generate" ; \
	  fi \
	fi

doxygen_dvi_install:
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
	  echo "removing apidoc/"; \
	  $(RM) -r apidoc/* ; rmdir apidoc ; \
	fi
	@if [ -d sourcedoc ]; then \
	  echo "removing sourcedoc/"; \
	  $(RM) -r sourcedoc/* ; rmdir sourcedoc ; \
	fi
	@if [ -r doxygen_api_conf ]; then \
	  echo "removing doxygen_api_conf"; $(RM) doxygen_api_conf ; \
	fi
	@if [ -r doxygen_full_conf ]; then \
	  echo "removing doxygen_full_conf"; $(RM) doxygen_full_conf ; \
	fi

	@if [ -r code_structure.html ]; then \
	  echo "removing code_structure.html"; $(RM) code_structure.html ; \
	fi
	@if [ -r mkdoc.pl ]; then \
	  echo "removing mkdoc.pl"; $(RM) mkdoc.pl ; \
	fi

	@if [ -r todo.html ]; then \
	  echo "removing todo.html"; $(RM) todo.html ; \
	fi
	@if [ -r todo2html.pl ]; then \
	  echo "removing todo2html.pl"; $(RM) todo2html.pl ; \
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
	@./mkdoc.pl $(top_srcdir) $@

todo.html: todo2html.pl
	@echo "Generating todo list"
	@./todo2html.pl $(top_srcdir)/TODO >todo.html

DOCPPFLAGS = --sort --no-java-graphs --c-comments --quantel \
             --upwards-arrows --no-define

%.dvi: %.tex docxx.sty
	@#Make it as if docxx is there, for VPATH builds.
	@if [ ! -r docxx.sty ] ; then \
	  echo "Sym-linking docxx.sty" ; \
	  ln -s $(srcdir)/docxx.sty . ; \
	fi
	@echo "Generating $@"
	@$(LATEX) "\scrollmode\input" "$(basename $@)" || :
	@$(LATEX) "\scrollmode\input" "$(basename $@)" || \
	  ( $(RM) $@ ; false )

%.ps: %.dvi
	$(DVIPS) -o $@ $<

##########################################
# Automatic dependencies for doc++ stuff #
##########################################

-include .deps-apidoc-html
-include .deps-apidoc-tex
-include .deps-sourcedoc-html
-include .deps-sourcedoc-tex

distclean-local: clean-local
	@if [ -e .deps-apidoc-html ] ; then \
	  echo "removing .deps-apidoc-html"; $(RM) .deps-apidoc-html ; \
	fi
	@if [ -e .deps-apidoc-tex ] ; then \
	  echo "removing .deps-apidoc-tex"; $(RM) .deps-apidoc-tex ; \
	fi
	@if [ -e .deps-sourcedoc-html ] ; then \
	  echo "removing .deps-sourcedoc-html"; $(RM) .deps-sourcedoc-html ; \
	fi
	@if [ -e .deps-sourcedoc-tex ] ; then \
	  echo "removing .deps-sourcedoc-tex"; $(RM) .deps-sourcedoc-tex ; \
	fi

# Get GENDEPS macro
include $(top_srcdir)/Makefile_common.am

#########################
# Rules for doc++ stuff #
#########################

APIDOC_SRC    = `find $(top_srcdir)/include \( -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c \) -print`
SOURCEDOC_SRC = `find $(top_srcdir) \( \( -name \*.h -or -name \*.cc -or -name \*.cpp -or -name \*.c -and -not -name acconfig.h \) -and -not -path \*/tests/\* \) -print`

.FORCE: doxygen_docs

doxygen_docs: apidoc/html/index.html sourcedoc/html/index.html

doxygen_api_conf: $(top_srcdir)/configure
doxygen_full_conf: $(top_srcdir)/configure

apidoc/html/index.html: doxygen_api_conf \
                        $(srcdir)/doxygen_api_header.html_tmpl \
			$(srcdir)/doxygen_api_footer.html_tmpl
	@echo "Generating API documentation with doxygen"
	@depends="$(APIDOC_SRC)"; destfile=".deps-apidoc-html"; $(GENDEPS)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) $(DOXYGEN_PATH) doxygen_api_conf

sourcedoc/html/index.html: doxygen_full_conf \
                        $(srcdir)/doxygen_full_header.html_tmpl \
			$(srcdir)/doxygen_full_footer.html_tmpl
	@echo "Generating full source documentation with doxygen"
	@depends="$(SOURCEDOC_SRC)"; destfile=".deps-sourcedoc-html"; $(GENDEPS)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) $(DOXYGEN_PATH) doxygen_full_conf

## apidoc/index.html:
## 	@echo "Generating HTML documentation for API"
## 	@depends="$(APIDOC_SRC)"; destfile=".deps-apidoc-html"; $(GENDEPS)
## 	@doc++ $(DOCPPFLAGS) --dir apidoc $(APIDOC_SRC)
## 
## apidoc.tex:
## 	@echo "Generating LaTeX documentation for API"
## 	@depends="$(APIDOC_SRC)"; destfile=".deps-apidoc-tex"; $(GENDEPS)
## 	@doc++ -t $(DOCPPFLAGS) --output $@ $(APIDOC_SRC)
## 
## sourcedoc/index.html:
## 	@echo "Generating HTML documentation for full source"
## 	@depends="$(SOURCEDOC_SRC)"; destfile=".deps-sourcedoc-html"; $(GENDEPS)
## 	@doc++ $(DOCPPFLAGS) --all --private --dir sourcedoc $(SOURCEDOC_SRC)
## 
## sourcedoc.tex:
## 	@echo "Generating LaTeX documentation for full source"
## 	@depends="$(SOURCEDOC_SRC)"; destfile=".deps-sourcedoc-tex"; $(GENDEPS)
## 	@doc++ -t $(DOCPPFLAGS) --private --output $@ $(SOURCEDOC_SRC)

##################################
# Generate images from dia files #
##################################

%.eps: %.dia
	dia -e $@ $<

%.png: %.eps
	pstoimg -aaliastext -antialias -center 0 -density 40 -depth 1 $<
