## Process this file with automake to produce Makefile.in

SUFFIXES = .tex .dvi .ps .eps .dia .png

HTMLFILES = index.html intro.html intro_ir.html userman.html \
        stemming.html porterstem.html using_stemmers.html quickstart.html \
	quartzdesign.html

# Files which should be put in the distribution by automake
EXTRA_DIST = mkdoc.pl.in todo2html.pl.in todo.xml \
        doxygen_api_conf.in doxygen_full_conf.in \
        $(HTMLFILES) \
        MatcherDesign Notes \
	docxx.sty \
	doxygen_api_header.html_tmpl \
	doxygen_api_footer.html_tmpl \
	doxygen_full_header.html_tmpl \
	doxygen_full_footer.html_tmpl \
	distributed.txt \
	coding_policy.txt

# Things to do if have perl installed
if HAVE_PERL
CREATED_DOCUMENTS1=code_structure.html
# and if XML::Parser and Text::Format is installed
if HAVE_PERL_XML_AND_TEXT
CREATED_DOCUMENTS2=todo.html todo-release.html
endif
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
all-local:
dist-hook: doxygen_api_docs doxygen_dist
install-data-local: doxygen_api_docs doxygen_install
else
all-local:
dist-hook: doxygen_dist
install-data-local: doxygen_install
endif

# Data to install, in the usual automake way
docdatadir   = $(datadir)/xapian
docdata_DATA = $(CREATED_DOCUMENTS1) $(CREATED_DOCUMENTS2) $(HTMLFILES)

###############################################
# How to put doxygen docs in the distribution #
###############################################
doxygen_dist: doxygen_html_dist

doxygen_html_dist:
	@if test -f apidoc/html/index.html ; then \
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
	@if test -f apidoc/html/index.html ; then \
	  echo "Installing API HTML documentation" ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc/html ; \
	  $(INSTALL_DATA) apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	elif test -f $(srcdir)/apidoc/html/index.html ; then \
	  echo "Installing API HTML documentation" ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc ; \
	  $(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc/html ; \
	  $(INSTALL_DATA) $(srcdir)/apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	else \
	  echo "NOT installing API HTML documentation: not present, and can't generate" ; \
	fi

doxygen_dvi_install:
	@if test -f apidoc.dvi ; then \
	  echo "Installing API DVI documentation" ; \
	  $(INSTALL_DATA) apidoc.dvi $(DESTDIR)$(docdatadir)/ ; \
	elif test -f $(srcdir)/apidoc.dvi ; then \
	  echo "Installing API DVI documentation" ; \
	  $(INSTALL_DATA) $(srcdir)/apidoc.dvi $(DESTDIR)$(docdatadir)/ ; \
	else \
	  echo "NOT installing API DVI documentation: not present, and can't generate" ; \
	fi

###############################
# How to clean up local files #
###############################

clean-local:
	rm -rf apidoc sourcedoc

CLEANFILES=\
       doxygen_api_conf \
       doxygen_full_conf \
       code_structure.html \
       mkdoc.pl \
       todo.html \
       todo-release.html \
       todo2html.pl

################################
# How to uninstall local files #
################################
uninstall-local:
	rm -rf $(DESTDIR)$(docdatadir)/apidoc
	rm -f $(DESTDIR)$(docdatadir)/apidoc.dvi

###############
# Extra rules #
###############

code_structure.html: mkdoc.pl
	@echo "Generating documentation"
	@./mkdoc.pl $(top_srcdir) $@

todo.html: todo2html.pl todo.xml
	@echo "Generating todo list"
	@if test -f "todo.xml"; then d1="." ; else d1="$(srcdir)/"; fi; \
	if test -f "todo2html.pl"; then d2="." ; else d2="$(srcdir)/"; fi; \
	$${d2}/todo2html.pl $${d1}/todo.xml >$@

todo-release.html: todo2html.pl todo.xml
	@echo "Generating release todo list"
	@if test -f "todo.xml"; then d1="." ; else d1="$(srcdir)/"; fi; \
	if test -f "todo2html.pl"; then d2="." ; else d2="$(srcdir)/"; fi; \
	$${d2}/todo2html.pl --release \* $${d1}/todo.xml >$@

.tex.dvi: docxx.sty
	@#Make it as if docxx is there, for VPATH builds.
	@if test ! -f docxx.sty ; then \
	  echo "Sym-linking docxx.sty" ; \
	  rm -f docxx.sty ; \
	  ln -s $(srcdir)/docxx.sty . ; \
	fi
	@echo "Generating $@"
	@$(LATEX) "\scrollmode\input" `echo $@|sed 's!.*/!!'` || :
	@$(LATEX) "\scrollmode\input" `echo $@|sed 's!.*/!!'` || \
	  ( rm -f $@ ; false )

.dvi.ps:
	$(DVIPS) -o $@ $<

##########################################
# Automatic dependencies for doc++ stuff #
##########################################

-include .deps-apidoc-html
-include .deps-apidoc-tex
-include .deps-sourcedoc-html
-include .deps-sourcedoc-tex

DISTCLEANFILES=\
	.deps-apidoc-html \
	.deps-apidoc-tex \
	.deps-sourcedoc-html \
	.deps-sourcedoc-tex

# Get GENDEPS macro
include $(top_srcdir)/Makefile_common.am

#########################
# Rules for doc++ stuff #
#########################

APIDOC_SRC    = `find $(top_srcdir)/include \( -name \*.h -o -name \*.cc -o -name \*.cpp -o -name \*.c \) -print`
SOURCEDOC_SRC = `find $(top_srcdir) \( \( -name \*.h -o -name \*.cc -o -name \*.cpp -o -name \*.c -not -name acconfig.h \) -not -path \*/tests/\* \) -print`

.FORCE: doxygen_source_docs doxygen_api_docs doxygen_docs

doxygen_source_docs: sourcedoc/html/index.html
doxygen_api_docs: apidoc/html/index.html
doxygen_docs: doxygen_source_docs doxygen_api_docs

doxygen_api_conf: $(top_srcdir)/configure.in
doxygen_full_conf: $(top_srcdir)/configure.in

apidoc/html/index.html: doxygen_api_conf \
                        $(srcdir)/doxygen_api_header.html_tmpl \
			$(srcdir)/doxygen_api_footer.html_tmpl
	@echo "Generating API documentation with doxygen"
	@depends="$(APIDOC_SRC)"; destfile=".deps-apidoc-html"; $(GENDEPS)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) $(DOXYGEN) doxygen_api_conf

sourcedoc/html/index.html: doxygen_full_conf \
                        $(srcdir)/doxygen_full_header.html_tmpl \
			$(srcdir)/doxygen_full_footer.html_tmpl
	@echo "Generating full source documentation with doxygen"
	@depends="$(SOURCEDOC_SRC)"; destfile=".deps-sourcedoc-html"; $(GENDEPS)
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) $(DOXYGEN) doxygen_full_conf

##################################
# Generate images from dia files #
##################################

.dia.eps:
	dia -e $@ $<

.eps.png:
	pstoimg -aaliastext -antialias -center 0 -density 40 -depth 1 $<
