## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/extra
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig

BUILT_SOURCES = xapian/xapian_wrap.cpp xapian/php_xapian.h xapian/xapian.php \
 xapian/Makefile.in xapian/config.m4 

# This target is not needed for make dist
all-local: xapian/configure
	cd xapian && ./configure && $(MAKE) install

install-exec-local:
	cd xapian && $(MAKE) install

xapian/configure: xapian/xapian_wrap.cpp xapian/Makefile.in config.m4 xapian/php_xapian.h xapian/xapian.php
	for file in $? ; do \
	  case "$$file" in xapian/*) ;; *) cp $$file xapian ;; esac ; \
	done
	cd xapian && $(PHPIZE)

xapian/xapian_wrap.cpp xapian/Makefile.in xapian/config.m4 xapian/php_xapian.h xapian/xapian.php: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i ../swig/om_util_php4.i
	test -d xapian || mkdir xapian
	$(SWIG) $(SWIG_INCLUDES) -c++ -php4 -phpfull -withlibs "xapian omqueryparser" -withincs xapian.h -noproxy -o xapian/xapian_wrap.cpp $(srcdir)/../swig/xapian.i

CLEANFILES = $(BUILT_SOURCES)
