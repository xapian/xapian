## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i

BUILT_SOURCES = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian/xapian.php \
 xapian/Makefile.in xapian/config.m4 

# For PHP4, SWIG generates a mini autoconf project in the xapian subdirectory.
# We generate and build this for "make all", and install it for "make install".
all-local: xapian/configure
	cd xapian &&\
	 ./configure --with-xapian="`echo $(XAPIAN_LIBS)|sed 's![^/]*$!!'`" &&\
	 $(MAKE)

install-local:
	cd xapian && $(MAKE) install

xapian/configure: xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php
	cd xapian && $(PHPIZE)

xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php: ../xapian.i util.i
	test -d xapian || mkdir xapian
	$(SWIG) -I. -c++ -php4 -phpfull -withlibs "xapian omqueryparser" -withincs "xapian.h xapian/queryparser.h" -noproxy -o xapian/xapian_wrap.cpp $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES)
