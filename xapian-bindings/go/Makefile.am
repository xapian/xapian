.NOTPARALLEL:

TESTS = ./test
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)'
	export abs_builddir ;\
	export srcdir ;
LOG_COMPILER = ${GO} test ./test

install-data-hook:
	cd setup && ${GO} run setup.go "install-bindings"

all : build
	cd setup && ${GO} run setup.go "xapian_bindings-->$(abs_top_builddir)" "xapian_cxxflags-->${CXXFLAGS}" \
	"xapian_cppflags-->${CPPFLAGS}" "lcflags-->${USE_CLANG_GO_LIB}"

build :
	${SWIG} ${SWIG_FLAGS} -c++ -go -cgo -intgosize 64 go.i
	${GO} env -w CXX="${CXX}"

check :
	${GO} test ./test

clean :
	rm -r xapian.go
	rm -r go_*
.PHONY: build run check
