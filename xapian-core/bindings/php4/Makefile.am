## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/extra
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig

BUILT_SOURCES = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian/xapian.php \
 xapian/Makefile.in xapian/config.m4 

# not called by "make all" or "make install" - this needs to be run after
# Xapian is installed unfortunately.
all-php4: xapian/configure
	cd xapian && ./configure --with-xapian="$(prefix)" && $(MAKE)

# not called by "make all" or "make install" - this needs to be run after
# Xapian is installed unfortunately.
install-php4: all-php4
	cd xapian && $(MAKE) install

xapian/configure: xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php
	cd xapian && $(PHPIZE)

xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i ../swig/om_util_php4.i
	test -d xapian || mkdir xapian
	$(SWIG) $(SWIG_INCLUDES) -c++ -php4 -phpfull -withlibs "xapian omqueryparser" -withincs "xapian.h xapian/queryparser.h" -noproxy -o xapian/xapian_wrap.cpp $(srcdir)/../swig/xapian.i

CLEANFILES = $(BUILT_SOURCES)
