## Process this file with automake to produce Makefile.in

noinst_HEADERS = omega.h query.h cgiparam.h symboltab.h

# include lex/flex generated C++ source
EXTRA_DIST = parsequery.cc

INCLUDES = $(LIBOMUS_CFLAGS)

bin_PROGRAMS = omega

omega_SOURCES = omega.cc query.cc cgiparam.cc parsequery.ll
omega_LDADD = @LIBOMUS_LIBS@

parsequery.cc: parsequery.ll
	$(SHELL) $(YLWRAP) "$(LEX)" $< lex.qlex.c $@ -- $(AM_LFLAGS) $(LFLAGS)
