## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

noinst_HEADERS = main.h query.h cgiparam.h symboltab.h

# include lex/flex generate C++ source
EXTRA_DIST = parsepage.cc parsequery.cc

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = ompf

ompf_SOURCES = main.cc query.cc cgiparam.cc parsepage.ll parsequery.ll
ompf_LDADD = ../libom.la

parsequery.cc: parsequery.ll
	$(SHELL) $(YLWRAP) "$(LEX)" $< lex.qlex.c $@ -- $(AM_LFLAGS) $(LFLAGS)
