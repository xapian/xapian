## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

EXTRA_DIST = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php \
 xapian/Makefile.in xapian/config.m4 

# This target is not needed for make dist
xapian: xapian/configure
	cd xapian && \
	./configure && \
	make

xapian/configure: xapian/config.m4 xapian/Makefile.in xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php
	for file in $? ; \
	do \
	  if [ "`dirname $$file`" != "xapian" ] ; \
	  then cp $$file xapian ; \
	  fi ; \
	done && \
	cd xapian && phpize

xapian/%_wrap.cpp xapian/php_%.h %.php: ../swig/%.i ../swig/omstem.i ../swig/om_util.i
	test -d xapian || mkdir xapian && \
	swig $(SWIG_INCLUDES) -c++ -php4 -nosync -shadow -o $@ $<
