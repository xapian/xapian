## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omuscat.so omuscat.pm

omstem_wrap.cc: ../swig/omstem.i
	swig $(SWIG_INCLUDES) -c++ -perl5 -shadow -o $@ $<

omuscat_wrap.cc: ../swig/omuscat.i ../swig/om_util.i ../swig/om_util_perl5.i
	swig $(SWIG_INCLUDES) -c++ -perl5 -shadow -o $@ $<

omstem.pm: omstem_wrap.cc
omstem.pm: omstem_wrap.cc

omuscat.pm: omuscat_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/lib/perl5/5.005/i386-linux/CORE/  -o $@

omuscat_wrap.o: omuscat_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/lib/perl5/5.005/i386-linux/CORE/  -o $@

omstem.so: omstem_wrap.o
	$(CXX) -shared $< -o $@

omuscat.so: omuscat_wrap.o
	$(CXX) -shared $< -o $@




