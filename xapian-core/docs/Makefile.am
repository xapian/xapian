## Process this file with automake to produce Makefile.in

# Note: we don't want dist tarballs built with docs missing, so for simplicity
# this Makefile.am blindly assumes the tools are in place when "make dist" is
# being run.  For "make all", "make check", etc we carefully skip any docs
# which we don't have the tools to generate.

# FIXME: add "xapian hacker docs" option and build sourcedoc.ps and HTML

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

# Things to do if have doxygen installed
if HAVE_DOXYGEN
all-local: doxygen_api_docs
else
all-local:
endif

# Data to install, in the usual automake way
docdatadir   = $(datadir)/xapian
docdata_DATA = $(HTMLDOCS) $(TEXTDOCS) apidoc.ps

dist-hook:
	$(mkinstalldirs) $(distdir)/apidoc
	cp -a apidoc/* $(distdir)/apidoc/

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc
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

MAINTAINERCLEANFILES=\
	doxygen_api_conf \
	doxygen_full_conf \
	code_structure.html \
	todo.html \
	todo-release.html

code_structure.html: mkdoc.pl
	$(PERL) $(srcdir)/mkdoc.pl $(top_srcdir) $@

todo.html: todo2html.pl todo.xml
	$(PERL) $(srcdir)/todo2html.pl $(srcdir)/todo.xml >$@

todo-release.html: todo2html.pl todo.xml
	$(PERL) $(srcdir)/todo2html.pl --release \* $(srcdir)/todo.xml >$@

apidoc.ps: apidoc/latex/refman.tex
	cd apidoc/latex && $(LATEX) "\scrollmode\input" refman.tex\
		&& $(DVIPS) -o ../../apidoc.ps refman.dvi

sourcedoc.ps: sourcedoc/latex/refman.tex
	cd sourcedoc/latex && $(LATEX) "\scrollmode\input" refman.tex\
		&& $(DVIPS) -o ../../sourcedoc.ps refman.dvi

#########################
# Rules for doxygen stuff #
#########################

.FORCE: doxygen_source_docs doxygen_api_docs doxygen_docs

doxygen_source_docs: sourcedoc/html/index.html
doxygen_api_docs: apidoc/html/index.html
doxygen_docs: doxygen_source_docs doxygen_api_docs

doxygen_api_conf: $(top_srcdir)/configure.in
doxygen_full_conf: $(top_srcdir)/configure.in

apidoc/html/index.html: doxygen_api_conf \
		$(srcdir)/doxygen_api_header.html_tmpl \
		$(srcdir)/doxygen_api_footer.html_tmpl \
		$(APIDOC_SRC)
	@echo "Generating API documentation with doxygen"
	top_srcdir=$(top_srcdir) $(DOXYGEN) doxygen_api_conf

sourcedoc/html/index.html: doxygen_full_conf \
		$(srcdir)/doxygen_full_header.html_tmpl \
		$(srcdir)/doxygen_full_footer.html_tmpl \
		$(SOURCEDOC_SRC)
	@echo "Generating full source documentation with doxygen"
	top_srcdir=$(top_srcdir) $(DOXYGEN) doxygen_full_conf

##################################
# Generate images from dia files #
##################################

.dia.eps:
	dia -e $@ $<

.eps.png:
	pstoimg -aaliastext -antialias -center 0 -density 40 -depth 1 $<
