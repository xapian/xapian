## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

EXTRA_DIST = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php \
 xapian/Makefile.in xapian/config.m4 xapian/swigphp.h xapian/xapian.cpp

#xapian/phpized: xapian/config.m4 xapian/Makefile.in xapian/swigphp.h xapian/xapian.cpp
#	: #test -d xapian || mkdir xapian
#	cd xapian && phpize
#	touch xapian/phpized

xapian/xapian_wrap.cpp xapian/php_xapian.h xapian.php: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -php4 -nosync -shadow \
	 -o xapian/xapian_wrap.cpp ../swig/xapian.i
