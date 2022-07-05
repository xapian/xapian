.NOTPARALLEL:

TESTS = ./test
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)'
	export abs_builddir ;\
	export srcdir ;
LOG_COMPILER = ${GO} test ./test

install-data-hook:
	cd setup && ${GO} run setup.go "without-core" "install" "cxxflags-->${CXXFLAGS}" \
	"cppflags-->${CPPFLAGS}" "lcflags-->${USE_CLANG_GO_LIB}" "xapian_cxxflags-->${GO_CXX}" \
	"xapian_libs-->${GO_LIB}"

all : build
	cd setup && ${GO} run setup.go "without-core" "build" "cxxflags-->${CXXFLAGS}" \
	"cppflags-->${CPPFLAGS}" "lcflags-->${USE_CLANG_GO_LIB}" "xapian_cxxflags-->${GO_CXX}" \
	"xapian_libs-->${GO_LIB}"

build :
	${SWIG} ${SWIG_FLAGS} -c++ -go -cgo -intgosize 64 raw/go.i
	${GO} env -w CXX="${CXX}"

check :
	${GO} test ./test

clean :
	rm -f raw/xapian.go
	rm -f raw/go_*
.PHONY: build run check
