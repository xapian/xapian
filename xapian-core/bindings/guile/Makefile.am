## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omstem.so

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i ../swig/om_util_guile.i
	swig $(SWIG_INCLUDES) -c++ -guile -o $@ $<

omseek_wrap.cc: ../swig/omsee.i ../swig/omstem.i ../swig/om_util.i ../swig/om_util_guile.i
	swig $(SWIG_INCLUDES) -c++ -guile -shadow -o $@ $<

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

omsee_wrap.o: omsee_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libomus.la -o $@

omseec.so: omsee_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libomus.la -o $@
