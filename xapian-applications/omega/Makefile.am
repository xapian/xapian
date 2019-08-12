## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

SUBDIRS = . docs

my-html-tok.h: gen-myhtmltags Makefile.am common/Tokeniseise.pm myhtmlparse.tokens
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/gen-myhtmltags' '$(srcdir)' < '$(srcdir)/myhtmlparse.tokens'

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

BUILT_SOURCES = extra/omegascript.vim my-html-tok.h mimemap.h namedents.h

MAINTAINERCLEANFILES = my-html-tok.h mimemap.h mimemap.stamp namedents.h

EXTRA_PROGRAMS = omindex_poppler omindex_libebook omindex_libetonyek omindex_tesseract omindex_gmime

EXTRA_DIST =\
	clickmodel/testdata/test1.log\
	clickmodel/testdata/test2.log\
	clickmodel/testdata/test3.log\
	common/Tokeniseise.pm \
	extra/omegascript.vim \
	extra/omegascript.vim.in \
	config.rpath \
	gen-mimemap \
	gen-myhtmltags \
	gen-namedents \
	gen-omegascript-vim \
	lsan.supp \
	mimemap.h \
	mimemap.stamp \
	mimemap.tokens \
	my-html-tok.h \
	myhtmlparse.tokens \
	namedents.h \
	templates/query \
	templates/topterms \
	templates/opensearch \
	templates/godmode \
	templates/xml \
	templates/inc/anyalldropbox \
	templates/inc/anyallradio \
	templates/inc/toptermsjs \
	xapian-omega.spec

AM_CPPFLAGS = \
	-I$(top_srcdir)/common \
	-DCONFIGFILE_SYSTEM=\"$(sysconfdir)/omega.conf\" \
	-DPKGLIBBINDIR=\"$(pkglibbindir)\"
AM_CXXFLAGS += $(XAPIAN_CXXFLAGS)

pkglibbindir = $(pkglibdir)/bin
pkglibbin_PROGRAMS = omega $(OMINDEX_MODULES)
dist_pkglibbin_SCRIPTS = mhtml2html outlookmsg2html rfc822tohtml vcard2text
bin_PROGRAMS = omindex scriptindex omindex-list generate-qrel-file
dist_bin_SCRIPTS = dbi2omega htdig2omega mbox2omega postprocess

check_PROGRAMS = atomparsetest csvesctest htmlparsetest jsonesctest md5test sdbntest urlenctest utf8converttest
dist_check_SCRIPTS = omegatest sourcetest
TESTS =	atomparsetest$(EXEEXT)\
	csvesctest$(EXEEXT)\
	htmlparsetest$(EXEEXT)\
	jsonesctest$(EXEEXT)\
	md5test$(EXEEXT)\
	omegatest\
	sdbntest$(EXEEXT)\
	sourcetest\
	urlenctest$(EXEEXT)\
	utf8converttest$(EXEEXT)

omegadatadir = $(datadir)/omega
dist_omegadata_DATA = htdig2omega.script mbox2omega.script

dist_sysconf_DATA = omega.conf

noinst_HEADERS = omega.h query.h cgiparam.h index_file.h mime.h\
 myhtmlparse.h htmlparse.h utils.h configfile.h date.h datetime.h\
 commonhelp.h cdb.h cdb_int.h failed.h fields.h hashterm.h jsonescape.h\
 loadfile.h md5.h md5wrap.h xmlparse.h metaxmlparse.h values.h utf8convert.h\
 namedents.h pkglibbindir.h datevalue.h sample.h strcasecmp.h\
 utf8truncate.h diritor.h runfilter.h freemem.h xpsxmlparse.h transform.h\
 weight.h expand.h svgparse.h tmpdir.h urldecode.h urlencode.h unixperm.h atomparse.h\
 xlsxparse.h opendocparse.h msxmlparse.h sort.h timegm.h csvescape.h\
 portability/mkdtemp.h portability/strptime.h\
 clickmodel/simplifieddbn.h clickmodel/session.h worker.h worker_comms.h handler.h

# headers maintained in xapian-core
noinst_HEADERS +=\
	common/alignment_cast.h\
	common/append_filename_arg.h\
	common/closefrom.h\
	common/gnu_getopt.h\
	common/keyword.h\
	common/msvc_dirent.h\
	common/omassert.h\
	common/overflow.h\
	common/parseint.h\
	common/realtime.h\
	common/safedirent.h\
	common/safefcntl.h\
	common/safesysselect.h\
	common/safesysstat.h\
	common/safesyswait.h\
	common/safeunistd.h\
	common/safewindows.h\
	common/safewinsock2.h\
	common/setenv.h\
	common/stdclamp.h\
	common/str.h\
	common/stringutils.h

AM_LDFLAGS = $(NO_UNDEFINED) $(MAGIC_LDFLAGS)

# We want to compile transform.cc with PCRE_CFLAGS, but if this adds a -I for
# a directory with a different version of iconv, then we get problems (this
# has been reported on macOS).  We solve this using a technique suggested by
# the automake manual: compile each of transform.cc and utf8convert.cc into
# its own convenience library so each only gets its own CPPFLAGS.
noinst_LTLIBRARIES = libtransform.la libutf8convert.la

libtransform_la_SOURCES = transform.cc
libtransform_la_CPPFLAGS = $(PCRE_CFLAGS)
libtransform_la_LIBADD = $(PCRE_LIBS)

libutf8convert_la_SOURCES = utf8convert.cc
libutf8convert_la_CPPFLAGS = $(AM_CPPFLAGS) $(INCICONV)
libutf8convert_la_LIBADD = $(XAPIAN_LIBS) $(LTLIBICONV)

omega_SOURCES = omega.cc query.cc cgiparam.cc utils.cc configfile.cc date.cc\
 cdb_init.cc cdb_find.cc cdb_hash.cc cdb_unpack.cc jsonescape.cc loadfile.cc\
 datevalue.cc common/str.cc sample.cc sort.cc urlencode.cc weight.cc expand.cc\
 csvescape.cc timegm.cc md5.cc md5wrap.cc
# Not currently used, and we ought to handle non-Unix perms too: unixperm.cc
omega_LDADD = $(XAPIAN_LIBS) libtransform.la

omindex_SOURCES = omindex.cc myhtmlparse.cc htmlparse.cc index_file.cc mime.cc\
 common/getopt.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc md5.cc\
 md5wrap.cc xmlparse.cc metaxmlparse.cc sample.cc diritor.cc\
 runfilter.cc freemem.cc common/msvc_dirent.cc xpsxmlparse.cc common/str.cc\
 pkglibbindir.cc svgparse.cc tmpdir.cc urlencode.cc atomparse.cc xlsxparse.cc\
 opendocparse.cc common/keyword.cc msxmlparse.cc common/safe.cc timegm.cc\
 datetime.cc common/closefrom.cc worker.cc worker_comms.cc
if NEED_MKDTEMP
omindex_SOURCES += portability/mkdtemp.cc
endif
omindex_LDADD = $(MAGIC_LIBS) $(XAPIAN_LIBS) $(ZLIB_LIBS) libutf8convert.la

omindex_poppler_SOURCES = assistant.cc worker_comms.cc common/str.cc handler_poppler.cc
omindex_poppler_LDADD = $(POPPLER_LIBS)
omindex_poppler_CXXFLAGS = $(POPPLER_CFLAGS)

omindex_libebook_SOURCES = assistant.cc worker_comms.cc handler_libebook.cc
omindex_libebook_LDADD = $(LIBEBOOK_LIBS)
omindex_libebook_CXXFLAGS = $(LIBEBOOK_CFLAGS) $(XAPIAN_CXXFLAGS)

omindex_libetonyek_SOURCES = assistant.cc worker_comms.cc handler_libetonyek.cc
omindex_libetonyek_LDADD = $(LIBETONYEK_LIBS)
omindex_libetonyek_CXXFLAGS = $(LIBETONYEK_CFLAGS) $(XAPIAN_CXXFLAGS)

omindex_tesseract_SOURCES = assistant.cc worker_comms.cc handler_tesseract.cc
omindex_tesseract_LDADD = $(TESSERACT_LIBS)
omindex_tesseract_CXXFLAGS = $(TESSERACT_CFLAGS)

omindex_gmime_SOURCES = assistant.cc worker_comms.cc myhtmlparse.cc datetime.cc htmlparse.cc common/keyword.cc handler_gmime.cc
omindex_gmime_LDADD = $(GMIME_LIBS) libutf8convert.la
omindex_gmime_CXXFLAGS = $(GMIME_CFLAGS) $(XAPIAN_CXXFLAGS)

scriptindex_SOURCES = scriptindex.cc myhtmlparse.cc htmlparse.cc\
 common/getopt.cc common/str.cc commonhelp.cc utils.cc hashterm.cc loadfile.cc\
 utf8truncate.cc\
 common/keyword.cc timegm.cc datetime.cc
if NEED_STRPTIME
scriptindex_SOURCES += portability/strptime.cc
endif
scriptindex_LDADD = $(XAPIAN_LIBS) libutf8convert.la

omindex_list_SOURCES = common/getopt.cc omindex-list.cc
omindex_list_LDADD = $(XAPIAN_LIBS)

generate_qrel_file_SOURCES = generate-qrel-file.cc clickmodel/simplifieddbn.cc\
 common/getopt.cc
generate_qrel_file_LDADD = $(XAPIAN_LIBS)

atomparsetest_SOURCES = atomparsetest.cc atomparse.cc htmlparse.cc\
 myhtmlparse.cc common/keyword.cc datetime.cc timegm.cc
atomparsetest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS) libutf8convert.la

htmlparsetest_SOURCES = htmlparsetest.cc myhtmlparse.cc htmlparse.cc\
 common/keyword.cc datetime.cc timegm.cc
htmlparsetest_LDADD = $(NO_INSTALL) $(XAPIAN_LIBS) libutf8convert.la

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
	$(MAKE) check SCRIPTINDEX='${bindir}/scriptindex' OMEGA='${pkglibbindir}/omega'
