## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omstem.so

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i ../swig/om_util_tcl8.i
	swig $(SWIG_INCLUDES) -c++ -tcl8 -o $@ $(srcdir)/../swig/omstem.i

xapian_wrap.cc: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $(srcdir)/../swig/xapian.i

xapian.py: xapian_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(srcdir)/omstem_wrap.cc -o $@

xapian_wrap.o: xapian_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(srcdir)/xapian_wrap.cc -I/usr/include/python1.5/ -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) -shared omstem_wrap.o ../../libxapian.la -o $@

xapianc.so: xapian_wrap.o
	$(LIBTOOL) $(CXX) -shared xapian_wrap.o ../../libxapian.la -o $@
