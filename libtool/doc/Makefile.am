## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = gnits
info_TEXINFOS = libtool.texi
libtool_TEXINFOS = PLATFORMS fdl.texi notes.texi
EXTRA_DIST = notes.txt

all-local: $(srcdir)/notes.txt

$(srcdir)/notes.txt: notes.texi
		cd $(srcdir) && \
		$(MAKEINFO) --no-headers $(MAKEINFOFLAGS) -o notes.txt notes.texi

