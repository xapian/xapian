## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include
PHP_INTERNAL_INCLUDES = -I/usr/include/php/Zend/

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omstem.so

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i ../swig/om_util_php4.i
	swig $(SWIG_INCLUDES) -c++ -php4 -o $@ $<

xapian_wrap.cc: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -php4 -shadow -o $@ $<

xapian.py: xapian_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PHP_INTERNAL_INCLUDES) -o $@

xapian_wrap.o: xapian_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PERL_INTERNAL_INCLUDES) -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libxapian.la -o $@

xapianc.so: xapian_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libxapian.la -o $@
