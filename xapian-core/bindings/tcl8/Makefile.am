## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omstem.so

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i ../swig/om_util_tcl8.i
	swig $(SWIG_INCLUDES) -c++ -tcl8 -o $@ $<

omsee_wrap.cc: ../swig/omsee.i ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $<

omsee.py: omsee_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

omsee_wrap.o: omsee_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/include/python1.5/ -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libomus.la -o $@

omseec.so: omsee_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libomus.la -o $@
