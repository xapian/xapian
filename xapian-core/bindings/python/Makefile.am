## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

wrap_omstem.c: ../swig/omstem.i
	swig -c++ -python ../swig/omstem.i
