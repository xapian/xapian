## Process this file with automake to produce Makefile.in

noinst_HEADERS = main.h query.h cgiparam.h symboltab.h

# include lex/flex generated C++ source
EXTRA_DIST = parsequery.cc

INCLUDES = $(LIBOMUS_CFLAGS)

bin_PROGRAMS = ompf

ompf_SOURCES = main.cc query.cc cgiparam.cc parsequery.ll
ompf_LDADD = @LIBOMUS_LIBS@

parsequery.cc: parsequery.ll
	$(SHELL) $(YLWRAP) "$(LEX)" $< lex.qlex.c $@ -- $(AM_LFLAGS) $(LFLAGS)
