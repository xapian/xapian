## Process this file with automake to produce Makefile.in

# Note: we don't want dist tarballs built with docs missing, so for simplicity
# this Makefile.am blindly assumes the tools are in place when "make dist" is
# being run.  For "make all", "make check", etc we carefully skip any docs
# which we don't have the tools to generate.

# FIXME: add "xapian hacker docs" option to build sourcedoc.ps and HTML

SUFFIXES = .eps .dia .png

HTMLDOCS = index.html intro.html intro_ir.html userman.html \
	stemming.html porterstem.html using_stemmers.html quickstart.html \
	quartzdesign.html \
	code_structure.html todo.html todo-release.html

# docs for developers
TEXTDOCS = MatcherDesign OMAims distributed.txt omsettings bm25 tests.txt

# Files which should be put in the distribution by automake
EXTRA_DIST = mkdoc.pl todo2html.pl todo.xml \
	doxygen_api_conf.in doxygen_full_conf.in \
       	$(HTMLDOCS) $(TEXTDOCS) \
	doxygen_api_header.html_tmpl \
	doxygen_api_footer.html_tmpl \
	doxygen_full_header.html_tmpl \
	doxygen_full_footer.html_tmpl \
	apidoc.ps

# Build API docs with doxygen
all-local: doxygen_api_docs

# Data to install, in the usual automake way
docdatadir   = $(datadir)/xapian
docdata_DATA = $(HTMLDOCS) $(TEXTDOCS) apidoc.ps

dist-hook:
	: # Don't allow "make dist" if dot (part of graphviz) isn't installed
	test YES == "@DOXYGEN_HAVE_DOT@"
	$(mkinstalldirs) $(distdir)/apidoc
	if test -f apidoc/html/index.html ; then \
	  cp -a apidoc/* $(distdir)/apidoc/ ; \
	else \
	  cp -a $(srcdir)/apidoc/* $(distdir)/apidoc/ ; \
	fi

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc/html
	if test -f apidoc/html/index.html ; then \
	  $(INSTALL_DATA) apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	else \
	  $(INSTALL_DATA) $(srcdir)/apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	fi

###############################
# How to clean up local files #
###############################

maintainer-clean-local:
	rm -rf apidoc sourcedoc

################################
# How to uninstall local files #
################################
uninstall-local:
	rm -rf $(DESTDIR)$(docdatadir)/apidoc

###############
# Extra rules #
###############

MAINTAINERCLEANFILES= doxygen_api_conf doxygen_full_conf \
	code_structure.html todo.html todo-release.html apidoc.ps sourcedoc.ps

code_structure.html: mkdoc.pl
	$(PERL) $(srcdir)/mkdoc.pl "$(VERSION)" $(top_srcdir) $@

todo.html: todo2html.pl todo.xml
	$(PERL) $(srcdir)/todo2html.pl $(srcdir)/todo.xml >$@

todo-release.html: todo2html.pl todo.xml
	$(PERL) $(srcdir)/todo2html.pl --release \* $(srcdir)/todo.xml >$@

#####################
# Rules for doxygen #
#####################

.FORCE: doxygen_source_docs doxygen_api_docs doxygen_docs

doxygen_source_docs: sourcedoc/html/index.html
doxygen_api_docs: apidoc/html/index.html
doxygen_docs: doxygen_source_docs doxygen_api_docs

doxygen_api_conf: $(top_srcdir)/configure.in
doxygen_full_conf: $(top_srcdir)/configure.in

# NB run LaTeX twice to resolve forward references (in general more passes
# may be required, but 2 will probably be fine for us)

apidoc/html/index.html apidoc.ps: $(top_srcdir)/configure.in \
		$(srcdir)/doxygen_api_header.html_tmpl \
		$(srcdir)/doxygen_api_footer.html_tmpl \
		$(srcdir)/doxygen_api_conf.in \
		$(APIDOC_SRC)
	@echo "Generating API documentation with doxygen"
	top_srcdir=$(top_srcdir) $(DOXYGEN) doxygen_api_conf
	cd apidoc/latex && $(LATEX) "\scrollmode\input" refman.tex\
		&& $(LATEX) "\scrollmode\input" refman.tex\
		&& $(DVIPS) -o ../../apidoc.ps refman.dvi

sourcedoc/html/index.html sourcedoc.ps: $(top_srcdir)/configure.in \
		$(srcdir)/doxygen_full_header.html_tmpl \
		$(srcdir)/doxygen_full_footer.html_tmpl \
		$(srcdir)/doxygen_full_conf.in \
		$(SOURCEDOC_SRC)
	@echo "Generating full source documentation with doxygen"
	top_srcdir=$(top_srcdir) $(DOXYGEN) doxygen_full_conf
	cd sourcedoc/latex && $(LATEX) "\scrollmode\input" refman.tex\
		&& $(LATEX) "\scrollmode\input" refman.tex\
		&& $(DVIPS) -o ../../sourcedoc.ps refman.dvi

##################################
# Generate images from dia files #
##################################

.dia.eps:
	dia -e $@ $<

.eps.png:
	pstoimg -aaliastext -antialias -center 0 -density 40 -depth 1 $<
