EXTRA_DIST +=\
	getopt/dir_contents\
       	getopt/Makefile

noinst_LTLIBRARIES += libgetopt.la

libgetopt_la_SOURCES =\
	getopt/getopt.cc
