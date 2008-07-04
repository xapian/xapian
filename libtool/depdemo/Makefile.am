## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies foreign

SUBDIRS = l1 l2 l3 l4

EXTRA_DIST = sysdep.h

bin_PROGRAMS = depdemo depdemo_static

depdemo_SOURCES = main.c
depdemo_LDADD = $(top_builddir)/l1/libl1.la $(top_builddir)/l2/libl2.la \
		$(top_builddir)/l4/libl4.la
depdemo_DEPENDENCIES = $(top_builddir)/l1/libl1.la \
		$(top_builddir)/l2/libl2.la $(top_builddir)/l4/libl4.la

depdemo_static_SOURCES = $(depdemo_SOURCES)
depdemo_static_LDADD = $(depdemo_LDADD)
depdemo_static_DEPENDENCIES = $(depdemo_DEPENDENCIES)
depdemo_static_LDFLAGS = $(STATIC)

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
