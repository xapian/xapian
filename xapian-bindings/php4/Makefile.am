## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i

BUILT_SOURCES = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian/xapian.php \
 xapian/Makefile.in xapian/config.m4 

# For PHP4, SWIG generates a mini autoconf project in the xapian subdirectory.
# We generate and build this for "make all", and install it for "make install".
install-hook:
	cd xapian && $(MAKE) install

all-local: xapian/php_xapian.la

xapian/php_xapian.la: xapian/configure
	: # FIXME: this isn't going to work with an uninstalled libxapian
	: # The whole "--with-xapian" concept is deeply flawed really
	cd xapian &&\
	 ./configure --with-xapian="`echo $(XAPIAN_CXXFLAGS)|sed 's!^-I!!;s!/include$$!!'`" &&\
	 $(MAKE)

xapian/configure: xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php
	cd xapian && $(PHPIZE)

xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php: ../xapian.i util.i extra.i
	test -d xapian || mkdir xapian
	$(SWIG) -I/usr/include -I. -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -php4 -phpfull -withlibs "xapian xapianqueryparser" -withincs "xapian.h xapian/queryparser.h" -noproxy -o xapian/xapian_wrap.cpp $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
