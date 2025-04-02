## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

SUBDIRS = . docs

html-tok.h: gen-htmltags Makefile.am common/Tokeniseise.pm htmlparser.tokens
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/gen-htmltags' '$(srcdir)' < '$(srcdir)/htmlparser.tokens'

# Recover from the removal of $@.  A full explanation of this is in the
# automake manual under the heading "Multiple Outputs".
make_many_locked = \
@if test -f $@; then :; else \
  trap 'rm -rf "$(stamp)-lck" "$(stamp)"' 1 2 13 15; \
  if mkdir '$(stamp)-lck' 2>/dev/null; then \
    rm -f '$(stamp)'; \
    $(MAKE) $(AM_MAKEFLAGS) '$(stamp)'; \
    result=$$?; rm -rf '$(stamp)-lck'; exit $$result; \
  else \
    while test -d '$(stamp)-lck'; do sleep 1; done; \
    test -f '$(stamp)'; \
  fi; \
fi

multitarget_begin = @rm -f $@-t; touch $@-t
multitarget_end = @mv -f $@-t $@

stamp = mimemap.stamp
mimemap.h docs/inc/ignored.rst docs/inc/mimetypes.rst: $(stamp)
	$(make_many_locked)
$(stamp): gen-mimemap Makefile.am common/Tokeniseise.pm mimemap.tokens
	$(multitarget_begin)
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/gen-mimemap' '$(srcdir)' < '$(srcdir)/mimemap.tokens'
	$(multitarget_end)

namedents.h: gen-namedents Makefile.am common/Tokeniseise.pm
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/gen-namedents' '$(srcdir)'

BUILT_SOURCES = extra/omegascript.vim html-tok.h mimemap.h namedents.h

MAINTAINERCLEANFILES = html-tok.h mimemap.h mimemap.stamp namedents.h

EXTRA_PROGRAMS =\
	omindex_poppler\
	omindex_libebook\
	omindex_libetonyek\
	omindex_tesseract\
	omindex_gmime\
	omindex_libarchive\
	omindex_libabw\
	omindex_libcdr\
	omindex_libextractor\
	omindex_libgepub\
	omindex_libmwaw\
	omindex_libreofficekit

EXTRA_DIST =\
	clickmodel/testdata/clicklog1\
	clickmodel/testdata/clicklog2\
	clickmodel/testdata/clicklog3\
	common/Tokeniseise.pm \
	extra/omegascript.vim \
	extra/omegascript.vim.in \
	config.rpath \
	gen-mimemap \
	gen-htmltags \
	gen-namedents \
	gen-omegascript-vim \
	mimemap.h \
	mimemap.stamp \
	mimemap.tokens \
	html-tok.h \
	htmlparser.tokens \
	namedents.h \
	templates/query \
	templates/topterms \
	templates/opensearch \
	templates/godmode \
	templates/xml \
	templates/inc/anyalldropbox \
	templates/inc/anyallradio \
	templates/inc/toptermsjs \
	testfiles/abw/Friendly-Letter.awt \
	testfiles/abw/macbeth.zabw \
	testfiles/abw/test.abw \
	testfiles/abw/test1.abw \
	testfiles/apple_works/test_draw.cwk \
	testfiles/apple_works/test_spreadsheet.cwk \
	testfiles/apple_works/test_word.cwk \
	testfiles/audio/file_example_OOG_1MG.ogg \
	testfiles/audio/file_example_WAV_1MG.wav \
	testfiles/cdr/test1.cdr \
	testfiles/cdr/test2.cdr \
	testfiles/email/html.eml \
	testfiles/email/text.eml \
	testfiles/epub/epub2test.epub \
	testfiles/epub/epub3test.epub \
	testfiles/fb2/hello.fb2 \
	testfiles/fb2/lang-name.fb2 \
	testfiles/fb2/lang2.fb2 \
	testfiles/image/Test1.gif \
	testfiles/image/Test2.pgm \
	testfiles/image/Test3.ppm \
	testfiles/image/Test4.tiff \
	testfiles/image/Test5.webp \
	testfiles/image/poster-2.jpg \
	testfiles/image/poster.jpg \
	testfiles/image/scan-page.png \
	testfiles/iwork/test-keynote.key \
	testfiles/iwork/test-pages.pages \
	testfiles/lrf/hello.lrf \
	testfiles/msxml/2sheets.xlsx \
	testfiles/msxml/Book.xlsx \
	testfiles/msxml/Doc.docx \
	testfiles/msxml/Nature.pptx \
	testfiles/msxml/vnd.ms-xpsdocument_xpstest.xps \
	testfiles/opendoc/blank.odt \
	testfiles/opendoc/corrupt_file.odp \
	testfiles/opendoc/draw.odg \
	testfiles/opendoc/presentation.odp \
	testfiles/opendoc/presentation_template.otp \
	testfiles/opendoc/spreadsheet.ods \
	testfiles/opendoc/spreadsheet_template.ots \
	testfiles/opendoc/test.odt \
	testfiles/opendoc/text.odt \
	testfiles/opendoc/text_template.ott \
	testfiles/pdb/PalmDOC-hello.pdb \
	testfiles/pdb/PeanutPress-hello.pdb \
	testfiles/pdb/test.pdb \
	testfiles/pdf/poppler.pdf \
	testfiles/staroffice/calc.sxc \
	testfiles/staroffice/calc_template.stc \
	testfiles/staroffice/presentation.sxi \
	testfiles/staroffice/presentation_template.sti \
	testfiles/staroffice/text.sxw \
	testfiles/staroffice/text_template.stw \
	testfiles/svg/diagram.svg \
	testfiles/svg/diagram.svgz \
	testfiles/test-csv.csv \
	testfiles/test-html.html \
	testfiles/plaintext/iso88591.txt \
	testfiles/plaintext/utf16be-bom.txt \
	testfiles/plaintext/utf16le-bom.txt \
	testfiles/plaintext/utf8-bom.txt \
	testfiles/plaintext/utf8.txt \
	testfiles/video/file_example_AVI_480_750kB.avi \
	testfiles/video/file_example_OGG_480_1_7mg.ogg \
	ubsan.supp

AM_CPPFLAGS = \
	-I$(top_srcdir)/common \
	-DCONFIGFILE_SYSTEM=\"$(sysconfdir)/omega.conf\" \
	-DPKGLIBBINDIR=\"$(pkglibbindir)\"
AM_CXXFLAGS += $(WERROR)
AM_CXXFLAGS += $(XAPIAN_CXXFLAGS) $(MAGIC_CFLAGS)

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega $(OMINDEX_MODULES)
dist_pkglibbin_SCRIPTS = mhtml2html outlookmsg2html rfc822tohtml vcard2text
bin_PROGRAMS = omindex scriptindex omindex-list generate-qrel-file
dist_bin_SCRIPTS = dbi2omega mbox2omega postprocess

check_PROGRAMS = atomparsertest csvesctest htmlparsertest jsonesctest md5test sdbntest urlenctest utf8converttest omindexcheck timecheck
dist_check_SCRIPTS = omegatest.pl sourcetest omindextest
TESTS_ENVIRONMENT = UBSAN_OPTIONS=suppressions=$(srcdir)/ubsan.supp $(EATMYDATA)
TESTS =	atomparsertest$(EXEEXT)\
	csvesctest$(EXEEXT)\
	htmlparsertest$(EXEEXT)\
	jsonesctest$(EXEEXT)\
	md5test$(EXEEXT)\
	omegatest.pl\
	omindextest\
	sdbntest$(EXEEXT)\
	sourcetest\
	urlenctest$(EXEEXT)\
	utf8converttest$(EXEEXT)
TEST_EXTENSIONS = .pl
PL_LOG_COMPILER = $(PERL)

omegadatadir = $(datadir)/omega
dist_omegadata_DATA = mbox2omega.script

dist_sysconf_DATA = omega.conf

noinst_HEADERS = omega.h query.h cgiparam.h index_file.h mime.h\
 htmlparser.h xmlparser.h utils.h configfile.h date.h datetime.h\
 commonhelp.h cdb.h cdb_int.h failed.h fields.h hashterm.h jsonescape.h\
 loadfile.h md5.h md5wrap.h abiwordparser.h opendocmetaparser.h values.h utf8convert.h\
 namedents.h pkglibbindir.h datevalue.h genericxmlparser.h sample.h strcasecmp.h\
 utf8truncate.h diritor.h runfilter.h freemem.h xpsparser.h transform.h\
 weight.h expand.h svgparser.h tmpdir.h urldecode.h urlencode.h unixperm.h atomparser.h\
 xlsxparser.h opendocparser.h msxmlparser.h sort.h\
 mkdtemp.h strptime.h timegm.h\
 csvescape.h\
 clickmodel/simplifieddbn.h clickmodel/session.h worker.h worker_comms.h handler.h

# headers maintained in xapian-core
noinst_HEADERS +=\
	common/alignment_cast.h\
	common/append_filename_arg.h\
	common/closefrom.h\
	common/gnu_getopt.h\
	common/keyword.h\
	common/msvc_dirent.h\
	common/negate_unsigned.h\
	common/omassert.h\
	common/overflow.h\
	common/parseint.h\
	common/realtime.h\
	common/safedirent.h\
	common/safefcntl.h\
	common/safesysexits.h\
	common/safesyssocket.h\
	common/safesysselect.h\
	common/safesysstat.h\
	common/safesyswait.h\
	common/safeunistd.h\
	common/safewindows.h\
	common/safewinsock2.h\
	common/setenv.h\
	common/str.h\
	common/stringutils.h

AM_LDFLAGS = $(NO_UNDEFINED)

# We want to compile transform.cc with PCRE2_CFLAGS, but if this adds a -I for
# a directory with a different version of iconv, then we get problems (this
# has been reported on macOS).  We solve this using a technique suggested by
# the automake manual: compile each of transform.cc and utf8convert.cc into
# its own convenience library so each only gets its own CPPFLAGS.
noinst_LTLIBRARIES = libtransform.la libutf8convert.la

libtransform_la_SOURCES = transform.cc
libtransform_la_CPPFLAGS = $(PCRE2_CFLAGS)
libtransform_la_LIBADD = $(PCRE2_LIBS)

libutf8convert_la_SOURCES = utf8convert.cc
libutf8convert_la_CPPFLAGS = $(AM_CPPFLAGS) $(INCICONV)
libutf8convert_la_LIBADD = $(XAPIAN_LIBS) $(LTLIBICONV)

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc\
 cdb_init.cc cdb_find.cc cdb_hash.cc cdb_unpack.cc jsonescape.cc loadfile.cc\
 datevalue.cc common/str.cc sample.cc sort.cc urlencode.cc weight.cc expand.cc\
 csvescape.cc timegm.cc md5.cc md5wrap.cc
# Not currently used, and we ought to handle non-Unix perms too: unixperm.cc
omega_LDADD = $(XAPIAN_LIBS) libtransform.la

omindex_SOURCES = omindex.cc htmlparser.cc xmlparser.cc index_file.cc mime.cc\
 common/getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc md5.cc\
 md5wrap.cc abiwordparser.cc opendocmetaparser.cc sample.cc diritor.cc\
 runfilter.cc freemem.cc common/msvc_dirent.cc xpsparser.cc common/str.cc\
 pkglibbindir.cc svgparser.cc tmpdir.cc urlencode.cc atomparser.cc xlsxparser.cc\
 opendocparser.cc common/keyword.cc msxmlparser.cc common/safe.cc\
 mkdtemp.cc strptime.cc timegm.cc\
 datetime.cc common/closefrom.cc worker.cc worker_comms.cc
omindex_LDADD = $(MAGIC_LIBS) $(XAPIAN_LIBS) $(ZLIB_LIBS) libutf8convert.la

omindex_poppler_SOURCES = assistant.cc worker_comms.cc common/str.cc handler_poppler.cc
omindex_poppler_LDADD = $(POPPLER_LIBS)
omindex_poppler_CXXFLAGS = $(AM_CXXFLAGS) $(POPPLER_CFLAGS)

omindex_libebook_SOURCES = assistant.cc worker_comms.cc handler_libebook.cc
omindex_libebook_LDADD = $(LIBEBOOK_LIBS)
omindex_libebook_CXXFLAGS = $(AM_CXXFLAGS) $(LIBEBOOK_CFLAGS)

omindex_libetonyek_SOURCES = assistant.cc worker_comms.cc handler_libetonyek.cc
omindex_libetonyek_LDADD = $(LIBETONYEK_LIBS)
omindex_libetonyek_CXXFLAGS = $(AM_CXXFLAGS) $(LIBETONYEK_CFLAGS)

omindex_libgepub_SOURCES = assistant.cc worker_comms.cc htmlparser.cc datetime.cc xmlparser.cc common/keyword.cc handler_libgepub.cc
omindex_libgepub_LDADD = $(LIBGEPUB_LIBS) libutf8convert.la
omindex_libgepub_CXXFLAGS = $(AM_CXXFLAGS) $(LIBGEPUB_CFLAGS)

omindex_tesseract_SOURCES = assistant.cc worker_comms.cc handler_tesseract.cc
omindex_tesseract_LDADD = $(TESSERACT_LIBS)
omindex_tesseract_CXXFLAGS = $(AM_CXXFLAGS) $(TESSERACT_CFLAGS)

omindex_gmime_SOURCES = assistant.cc worker_comms.cc htmlparser.cc datetime.cc xmlparser.cc common/keyword.cc handler_gmime.cc common/str.cc
omindex_gmime_LDADD = $(GMIME_LIBS) libutf8convert.la
omindex_gmime_CXXFLAGS = $(AM_CXXFLAGS) $(GMIME_CFLAGS)

omindex_libarchive_SOURCES = assistant.cc worker_comms.cc opendocmetaparser.cc opendocparser.cc datetime.cc xmlparser.cc htmlparser.cc msxmlparser.cc xlsxparser.cc xpsparser.cc common/keyword.cc common/str.cc handler_libarchive.cc
omindex_libarchive_LDADD = $(LIBARCHIVE_LIBS) libutf8convert.la
omindex_libarchive_CXXFLAGS = $(AM_CXXFLAGS) $(LIBARCHIVE_CFLAGS)

omindex_libabw_SOURCES = assistant.cc worker_comms.cc handler_libabw.cc
omindex_libabw_LDADD = $(LIBABW_LIBS)
omindex_libabw_CXXFLAGS = $(AM_CXXFLAGS) $(LIBABW_CFLAGS)

omindex_libcdr_SOURCES = assistant.cc worker_comms.cc handler_libcdr.cc
omindex_libcdr_LDADD = $(LIBCDR_LIBS)
omindex_libcdr_CXXFLAGS = $(AM_CXXFLAGS) $(LIBCDR_CFLAGS)

omindex_libextractor_SOURCES = assistant.cc worker_comms.cc handler_libextractor.cc
omindex_libextractor_LDADD = $(LIBEXTRACTOR_LIBS)
omindex_libextractor_CXXFLAGS = $(AM_CXXFLAGS) $(LIBEXTRACTOR_CFLAGS)

omindex_libmwaw_SOURCES = assistant.cc worker_comms.cc handler_libmwaw.cc common/str.cc
omindex_libmwaw_LDADD = $(LIBMWAW_LIBS)
omindex_libmwaw_CXXFLAGS = $(AM_CXXFLAGS) $(LIBMWAW_CFLAGS)

omindex_libreofficekit_SOURCES = assistant.cc worker_comms.cc datetime.cc loadfile.cc tmpdir.cc urlencode.cc xmlparser.cc htmlparser.cc common/keyword.cc common/str.cc handler_libreofficekit.cc
omindex_libreofficekit_LDADD = $(LIBREOFFICEKIT_LIBS) libutf8convert.la
omindex_libreofficekit_CPPFLAGS = $(AM_CPPFLAGS) $(LIBREOFFICEKIT_CPPFLAGS)
omindex_libreofficekit_CXXFLAGS = $(AM_CXXFLAGS) $(LIBREOFFICEKIT_CFLAGS)

scriptindex_SOURCES = scriptindex.cc\
 genericxmlparser.cc htmlparser.cc xmlparser.cc\
 common/getopt.cc common/str.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc\
 utf8truncate.cc\
 common/keyword.cc strptime.cc timegm.cc datetime.cc
scriptindex_LDADD = $(XAPIAN_LIBS) libutf8convert.la

omindex_list_SOURCES = common/getopt.cc omindex-list.cc
omindex_list_LDADD = $(XAPIAN_LIBS)

generate_qrel_file_SOURCES = generate-qrel-file.cc clickmodel/simplifieddbn.cc\
 common/getopt.cc
generate_qrel_file_LDADD = $(XAPIAN_LIBS)

atomparsertest_SOURCES = atomparsertest.cc atomparser.cc xmlparser.cc\
 htmlparser.cc common/keyword.cc datetime.cc timegm.cc
atomparsertest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS) libutf8convert.la

htmlparsertest_SOURCES = htmlparsertest.cc htmlparser.cc xmlparser.cc\
 common/keyword.cc datetime.cc timegm.cc
htmlparsertest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS) libutf8convert.la

md5test_SOURCES = md5test.cc md5wrap.cc md5.cc
md5test_LDADD = $(NO_INSTALL)

utf8converttest_SOURCES = utf8converttest.cc
utf8converttest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS) libutf8convert.la

urlenctest_SOURCES = urlenctest.cc urlencode.cc
urlenctest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS)

csvesctest_SOURCES = csvesctest.cc csvescape.cc
csvesctest_LDADD = $(NO_INSTALL)

sdbntest_SOURCES = clickmodel/tests/sdbntest.cc clickmodel/simplifieddbn.cc
sdbntest_LDADD = $(NO_INSTALL)

jsonesctest_SOURCES = jsonesctest.cc jsonescape.cc
jsonesctest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS)

omindexcheck_SOURCES = omindexcheck.cc
omindexcheck_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS)

timecheck_SOURCES = timecheck.cc
timecheck_LDADD = $(NO_INSTALL)

if !MAINTAINER_NO_DOCS
dist_man_MANS = omindex.1 scriptindex.1 omindex-list.1
MAINTAINERCLEANFILES += $(dist_man_MANS)
endif

if DOCUMENTATION_RULES
omindex.1: omindex$(EXEEXT) makemanpage
	./makemanpage ./omindex $(srcdir)/omindex.cc omindex.1

scriptindex.1: scriptindex$(EXEEXT) makemanpage
	./makemanpage ./scriptindex $(srcdir)/scriptindex.cc scriptindex.1

omindex-list.1: omindex-list$(EXEEXT) makemanpage
	./makemanpage ./omindex-list $(srcdir)/omindex-list.cc omindex-list.1
endif

if MAINTAINER_MODE
extra/omegascript.vim: extra/omegascript.vim.in query.cc configure.ac
	$(PERL) $(srcdir)/gen-omegascript-vim "$(srcdir)" "$(PACKAGE_VERSION)"
endif

DISTCHECK_CONFIGURE_FLAGS = "XAPIAN_CONFIG=$(XAPIAN_CONFIG)"

installcheck-local:
	$(MAKE) check \
	    OMEGA='${pkglibbindir}/omega' \
	    OMINDEX='${bindir}/omindex' \
	    SCRIPTINDEX='${bindir}/scriptindex' \
	    XAPIAN_OMEGA_PKGLIBBINDIR='${pkglibbindir}'
