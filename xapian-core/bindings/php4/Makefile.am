## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

EXTRA_DIST = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php \
 xapian/Makefile.in xapian/config.m4 

# This target is not needed for make dist
xapian: xapian/configure
	cd xapian &&\
	./configure &&\
	make

xapian/configure: xapian/config.m4 xapian/Makefile.in xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php
	for file in $? ; do \
	  case "$$file" in xapian/*) ;; *) cp $$file xapian ;; esac ; \
	done
	cd xapian && phpize

xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i
	test -d xapian || mkdir xapian
	swig $(SWIG_INCLUDES) -c++ -php4 -nosync -shadow -o $@ ../swig/xapian.i
