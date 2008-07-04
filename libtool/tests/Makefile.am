## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = gnits

CXX_TESTS = \
	tagdemo-static.test tagdemo-make.test tagdemo-exec.test \
	tagdemo-conf.test tagdemo-make.test tagdemo-exec.test \
	tagdemo-shared.test tagdemo-make.test tagdemo-exec.test

F77_TESTS = \
	f77demo-static.test f77demo-make.test f77demo-exec.test \
	f77demo-conf.test f77demo-make.test f77demo-exec.test \
	f77demo-shared.test f77demo-make.test f77demo-exec.test 

COMMON_TESTS = \
	cdemo-static.test cdemo-make.test cdemo-exec.test \
	demo-static.test demo-make.test demo-exec.test \
	demo-inst.test demo-unst.test \
	depdemo-static.test depdemo-make.test \
	depdemo-exec.test depdemo-inst.test depdemo-unst.test \
	mdemo-static.test mdemo-make.test mdemo-exec.test \
	mdemo-inst.test mdemo-unst.test \
	cdemo-conf.test cdemo-make.test cdemo-exec.test \
	demo-conf.test demo-make.test demo-exec.test \
	demo-inst.test demo-unst.test deplibs.test \
	depdemo-conf.test depdemo-make.test \
	depdemo-exec.test depdemo-inst.test depdemo-unst.test \
	mdemo-conf.test mdemo-make.test mdemo-exec.test \
	mdemo-inst.test mdemo-unst.test dryrun.test \
	demo-nofast.test demo-make.test demo-exec.test \
	demo-inst.test demo-unst.test \
	demo-pic.test demo-make.test demo-exec.test \
	demo-nopic.test demo-make.test demo-exec.test \
	depdemo-nofast.test depdemo-make.test \
	depdemo-exec.test depdemo-inst.test depdemo-unst.test \
	cdemo-shared.test cdemo-make.test cdemo-exec.test \
	demo-shared.test demo-make.test demo-exec.test demo-inst.test \
	hardcode.test build-relink.test noinst-link.test demo-unst.test \
	depdemo-shared.test depdemo-make.test \
	depdemo-exec.test depdemo-inst.test build-relink2.test \
	depdemo-unst.test \
	mdemo-shared.test mdemo-make.test mdemo-exec.test \
	mdemo-inst.test mdemo-unst.test \
	assign.test link.test link-2.test nomode.test \
	quote.test sh.test suffix.test pdemo-conf.test \
	pdemo-make.test pdemo-exec.test pdemo-inst.test \
	mdemo-conf.test mdemo-make.test mdemo2-conf.test \
	mdemo2-make.test mdemo2-exec.test \
	duplicate_members.test link-order.test

if HAVE_CXX
if HAVE_F77
TESTS = $(COMMON_TESTS) $(CXX_TESTS) $(F77_TESTS)
else
TESTS = $(COMMON_TESTS) $(CXX_TESTS)
endif
else
if HAVE_F77
TESTS = $(COMMON_TESTS) $(F77_TESTS)
else
TESTS = $(COMMON_TESTS)
endif
endif

demo-conf.test: $(top_builddir)/libtool

$(top_builddir)/libtool:
	cd $(top_builddir) && $(MAKE) all

# We should export these variables when we build the libtool script.
CFLAGS = @CFLAGS@
CPPFLAGS = @CPPFLAGS@
LDFLAGS = @LDFLAGS@
LIBS = @LIBS@
FFLAGS = @FFLAGS@

# Be sure to reexport important environment variables.
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
	CPP="$(CPP)" CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
	LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
	OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" CONFIG_SHELL="$(SHELL)" \
	CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" CXXCPP="$(CXXCPP)" \
	F77="$(F77)" FFLAGS="$(FFLAGS)"

EXTRA_DIST = defs $(COMMON_TESTS) $(CXX_TESTS) $(F77_TESTS)

# We need to remove any files that the above tests created.
clean-local:
	-test -f ../pdemo/Makefile && cd ../pdemo && $(MAKE) distclean
	-test -f ../cdemo/Makefile && cd ../cdemo && $(MAKE) distclean
	-test -f ../demo/Makefile && cd ../demo && $(MAKE) distclean
	-test -f ../depdemo/Makefile && cd ../depdemo && $(MAKE) distclean
	-test -f ../mdemo/Makefile && cd ../mdemo && $(MAKE) distclean
	-test -f ../tagdemo/Makefile && cd ../tagdemo && $(MAKE) distclean
	-test -f ../f77demo/Makefile && cd ../f77demo && $(MAKE) distclean
	rm -rf _inst .libs
