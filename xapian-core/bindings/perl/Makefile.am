## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/include

target: omstem.so omstem.pm

omstem_wrap.cc: ../swig/omstem.i
	swig -c++ -perl5 -shadow -o $@ $<

omstem.pm: omstem_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/lib/perl5/5.005/i386-linux/CORE/  -o $@

omstem.so: omstem_wrap.o
	$(CXX) 
