## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/
#SUBDIRS=xapian

target: xapian/phpized xapian/xapian_wrap.cpp 
	cd xapian && \
	./configure && \
	make

xapian/phpized: xapian/config.m4 xapian/Makefile.in xapian/swigphp.h xapian/xapian.cpp
	test -d xapian || mkdir xapian && \
	cp $? xapian && \
	cd xapian && \
	phpize && \
	touch phpized

xapian/xapian_wrap.cpp: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -php4 -shadow -o $@ $<

xapian/xapian.pm: xapian xapian_wrap.cpp

