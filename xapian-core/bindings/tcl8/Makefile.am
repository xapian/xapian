## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omstem.so

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i ../swig/om_util_tcl8.i
	swig $(SWIG_INCLUDES) -c++ -tcl8 -o $@ $<

xapian_wrap.cc: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $<

xapian.py: xapian_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

xapian_wrap.o: xapian_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/include/python1.5/ -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libxapian.la -o $@

xapianc.so: xapian_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libxapian.la -o $@
